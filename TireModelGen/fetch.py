import requests
import json
import base64
import struct
from pathlib import Path

URL = "http://server.finishlinebyner.com:8050/_dash-update-component"
HEADERS = {"Content-Type": "application/json"}
PARENT_PATH = Path(__file__).parent
BIN_PATH = PARENT_PATH / "daytona_600.bin"
PAYLOAD_PATH = PARENT_PATH / "payload.json"

HEADER_FORMAT = "<IBH16sf"
POINT_FORMAT = "<ff"
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)
POINT_SIZE = struct.calcsize(POINT_FORMAT)
MAGIC = 0x004E4552


def get_data():
    with open(PAYLOAD_PATH, "r") as f:
        payload = json.load(f)
    with requests.Session() as s:
        res = s.post(URL, json=payload, headers=HEADERS)
    data = res.json()
    data = res.json()
    y_data = data["response"]["tire-model-longitudinal-graph"]["figure"]["data"][0]["y"]
    x_data = data["response"]["tire-model-longitudinal-graph"]["figure"]["data"][0][
        "x"
    ]["bdata"]
    return x_data, y_data


def bdata_to_list(bdata):
    raw = base64.b64decode(bdata)
    n = len(raw) // 8
    fmt = "<" + "d" * n
    return list(struct.unpack(fmt, raw))


def save_bin(x, y, filename):
    """
    Save x and y data to a binary file.
    """
    points = list(zip(x, y))
    with open(filename, "wb") as f:
        # Write header
        f.write(
            struct.pack(
                HEADER_FORMAT, MAGIC, 1, len(points), b"tire_model", float(max(y))
            )
        )
        # Write points
        for x_val, y_val in points:
            f.write(struct.pack(POINT_FORMAT, float(x_val), float(y_val)))


if __name__ == "__main__":
    x_data, ydata = get_data()
    x_data: list = bdata_to_list(x_data)
    save_bin(x_data, ydata, BIN_PATH)
