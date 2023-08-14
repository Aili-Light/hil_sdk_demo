import argparse, sys
import numpy as np
import cv2
import glob
import os

def display_raw12(filepath, h, w):
    try:
        with open(filepath, 'rb') as fs:
            bytes = fs.read()
            p_array = np.frombuffer(bytes, dtype=np.uint16)
    except IOError:
        print("Failed to load image from [%s]!" % filepath)
    else:
        # print("Load image from [%s]" % filepath)
        image_scl = cv2.convertScaleAbs(p_array, alpha=0.0625, beta=0.0)
        img_in = image_scl.reshape((h, w, 1))
        image_rbg = cv2.cvtColor(img_in, cv2.COLOR_BayerBG2RGB)
        # filename = "image.bmp"
        # cv2.imwrite(filename, image_rbg)
        cv2.namedWindow("ImageDisplay", cv2.WINDOW_NORMAL)
        cv2.imshow("ImageDisplay", image_rbg)
        cv2.waitKey(100)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="RAW12 display"
    )
    parser.add_argument('--source_dir',
                        type=str,
                        help="image source directory",
                        required=True
    )
    parser.add_argument('--image_width',
                        type=int,
                        help="image width",
                        required=True
    )
    parser.add_argument('--image_height',
                        type=int,
                        help="image height",
                        required=True
    )

    args = parser.parse_args()
    source_dir = args.source_dir
    image_width = args.image_width
    image_height = args.image_height

    os.chdir(source_dir)
    dirs =  glob.glob("*.raw")
    for filepath in dirs:
        print("load file : ", filepath)
        display_raw12(filepath, image_height, image_width)
        dirs.append(filepath)