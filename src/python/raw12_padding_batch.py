import argparse, sys
import numpy as np
import cv2
import glob
import os

def convert_raw12_padding(filepath):
    try:
        with open(filepath, 'rb') as fs:
            bytes = fs.read()
            p_array = np.frombuffer(bytes, dtype=np.uint8)
    except IOError:
        print("Failed to load image from [%s]!" % filepath)
    else:
        # print("Load image from [%s]" % filepath)
        p_data = np.zeros(shape=(image_height*image_width, 1, 1), dtype=np.uint16)
        for i in range(0, int(image_height*image_width/2)):
            p_data[2*i] = (((np.ushort(p_array[3*i]) << 4) & 0x0FF0) | np.ushort((p_array[3*i+2] >> 0) & 0x000F))
            p_data[2*i+1] = (((np.ushort(p_array[3*i+1]) << 4) & 0x0FF0) | np.ushort((p_array[3*i+2] >> 4) & 0x000F))
        
        return p_data

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="RAW12 Padding"
    )
    parser.add_argument('--source_dir',
                        type=str,
                        help="image source directory",
                        required=True
    )
    parser.add_argument('--output_dir',
                        type=str,
                        help="output directory",
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
    output_dir = args.output_dir
    image_width = args.image_width
    image_height = args.image_height

    os.chdir(source_dir)
    for filepath in glob.glob("*.raw"):
        print("load file : ", filepath)
        p_data = convert_raw12_padding(filepath)
        foldername, filename = os.path.split(filepath)
        filename_h = (os.path.splitext(filename)[0])
        filename_t = (os.path.splitext(filename)[1])
        save_type = ".raw12p"
        file_path_save = os.path.join(output_dir, filename_h + save_type + filename_t)
        # print(file_path_save)
        with open(file_path_save, "wb") as binary_file:
            # Write bytes to file
            binary_file.write(p_data)