# coding = UTF8

def main():
    masks = [
        "f0f00f0ff0f00f0f",
        "0000f0f00000f0f0",
        "0f0f00000f0f0000",
        "ff00ff0000ff00ff",
        "00000000ff00ff00",
        "00ff00ff00000000",

        "000f000f000f000f",
        "00f000f000f000f0",
        "0f000f000f000f00",
        "f000f000f000f000",

        "000000000000ffff",
        "00000000ffff0000",
        "0000ffff00000000",
        "ffff000000000000"
    ]

    # results = []
    for mask in masks:
        result = ""
        for char in mask:
            # print(char)
            if char == 'f':
                result += "11111"
            elif char == '0':
                result += '00000'
            else:
                raise ValueError()
        print(result[:16] + " " + result[16:])
        print(hex(int(result[:16],2)), hex(int(result[16:],2)))

if __name__ == "__main__":
    main()
