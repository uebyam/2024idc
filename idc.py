import os
import time
from idc_common_func import \
    ser_read_with_timeout, make_serial, make_cam, make_cv, classify_now


def main():
    ser = make_serial("/dev/ttyUSB0", 9600)
    if (ser is None):
        exit(0)

    pdir = os.path.dirname(__file__)
    cam, task3cv, task3labels, task2cv, task2labels = make_cam(), *make_cv(
        pdir + "/firetemp/",
        "firetemp_model.tflite",
        "firetemp_labels.txt"
    ), *make_cv(
        pdir + "/human/",
        "human_model.tflite",
        "human_labels.txt"
    )

    # debug_preview(cam, task3cv, task3labels)
    print("Starting")

    while True:
        c, c_ln = ser_read_with_timeout(ser, 0.05, 0.05, 1)
        if (c_ln != 1):
            continue
        c = c[0]

        if (c == ord("D")):
            ser.write(b"Y")
            ser.flush()

            print("Task 3")

            cam.start()
            image, label_id, _ = classify_now(cam, task3cv, 0.8)
            print("Activated " + task3labels[label_id])

            [task3_temp, task3_fire, task3_what][label_id](ser)

            cam.stop()

            print()
        elif (c == ord("H")):
            print("Task 2")
            cam.start()

            image, label_id, _ = None, None, 0.8
            for i in range(11000):
                image, label_id, _ = classify_now(cam, task2cv, 0.8)
                print(str(i), end=": ")
                print(task2labels[label_id])
                time.sleep(0.1)

            cam.stop()

            print()
        else:
            print("Unknown task: " + str(c))


def task3_temp(ser):
    for i in range(100):
        time.sleep(1)
        ser.write(b"T")  # Temperature
        ser.flush()

        ln, ln_ln = ser_read_with_timeout(ser, 60, 0.02, 1)
        if (ln_ln != 1):
            print("Error: Timeout on temperature str length")
            return
        ln = ln[0]

        tstr, ln_tstr = ser_read_with_timeout(ser, 60, 0.02, ln)
        if (ln_tstr != ln):
            print("Error: Timeout on temperature str")
            return
        print("rcv Temperature: " + tstr.decode("UTF-8"))


def task3_fire(ser):
    ser.write(b"F")  # Fire
    ser.flush()


def task3_what(ser):
    ser.write(b"W")
    ser.flush()


if __name__ == "__main__":
    main()
