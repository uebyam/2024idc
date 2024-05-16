import time
import serial

import numpy as np
import cv2
from picamera2 import Picamera2
from libcamera import Transform
from tflite_runtime.interpreter import Interpreter


def load_labels(path):  # Read the labels from the text file as a Python list.
    with open(path, 'r') as f:
        return [line.strip() for i, line in enumerate(f.readlines())]


def set_input_tensor(interpreter, image):
    tensor_index = interpreter.get_input_details()[0]['index']
    input_tensor = interpreter.tensor(tensor_index)()[0]
    input_tensor[:, :] = image


def classify_image(interpreter, image, top_k=1):
    set_input_tensor(interpreter, image)

    interpreter.invoke()
    output_details = interpreter.get_output_details()[0]
    output = np.squeeze(interpreter.get_tensor(output_details['index']))

    scale, zero_point = output_details['quantization']
    output = scale * (output - zero_point)

    ordered = np.argpartition(-output, 1)
    return [(i, output[i]) for i in ordered[:top_k]][0]


# TODO: kinda inefficient
def ser_read_with_timeout(ser, timeout, unit, count):
    ct = 0
    out = bytearray()
    ln = 0
    while (ct < timeout and ln < count):
        if (ser.in_waiting == 0):
            time.sleep(unit)
            ct += unit
            continue
        c = ser.read(1)
        if (len(c) != 1):  # should never happen
            time.sleep(unit)
            ct += unit
            continue
        out.append(c[0])
        ln += 1
    return out, ln


def make_serial(dev, baud):
    ser = None

    try:
        ser = serial.Serial(dev, baud, timeout=1)
        ser.reset_input_buffer()
    except serial.serialutil.SerialException as se:
        print(se)

    return ser


def make_cam():
    cam = Picamera2()
    cam.configure(cam.create_preview_configuration({
        "format": 'XRGB8888',
        "size": (224, 224)
    }
    #transform=Transform(hflip=1, vflip=1)))
    ))
    return cam


def make_cv(prefix, model_path, label_path):
    interpreter = Interpreter(prefix + model_path)
    interpreter.allocate_tensors()

    labels = load_labels(prefix + label_path)

    return interpreter, labels


def classify_now(cam, cv, prob_thresh):
    image, label_id, prob = None, None, 0.0
    while (prob < prob_thresh):
        image = cam.capture_array("main")[:, :, :3]
        label_id, prob = classify_image(cv, image)
    return image, label_id, prob


def debug_preview(cam, cv, labels):
    cam.start(show_preview=True)
    while True:
        image = cam.capture_array("main")[:, :, :3]

        label_id, prob = classify_image(cv, image)
        label = labels[label_id]

        prob = np.round(prob * 100, 2)

        result_text = "{} ({})".format(label, prob)
        overlay = np.zeros((500, 500, 4), dtype=np.uint8)
        overlay[:80, :250] = (0, 0, 255, 54)  # blue background
        text_color = (255, 0, 0, 255)  # red text
        cv2.putText(overlay, result_text, (0, 50), cv2.FONT_HERSHEY_SIMPLEX,
                    1, text_color, 3)

        cam.set_overlay(overlay)

        print("Result: {} ({}), Accuracy: {}%".format(label, label_id, prob))
