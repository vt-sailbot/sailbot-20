import yaml
import os


def read_gain(path=None):
    """Reads the read interval from config.yml"""
    if path is None:
        path = os.path.dirname(os.path.abspath(__file__))
    with open(path + "/config.yml", "r") as yml:
        conf = yaml.load(yml)
        gain = conf["rudder gain"]

    return eval(gain)


def read_interval(path=None):
    """Reads the read interval from config.yml"""
    if path is None:
        path = os.path.dirname(os.path.abspath(__file__))
    with open(path + "/config.yml", "r") as yml:
        conf = yaml.load(yml)
        helm_interval = conf["autohelm interval"]

    return eval(helm_interval)
