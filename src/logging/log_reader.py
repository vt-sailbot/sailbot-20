import json
import pdb
from pprint import pprint as pp

class logReader():

    def __init__(self, infile):
        self.infile = infile
        self.lineno = 0
        self.read_header()
        self.read_configs()

    def read_header(self):
        with open(self.infile, 'r') as log_file:
            self.header = json.loads(log_file.readline())
        self.lineno += 1

    def read_configs(self):
        num_config_files = self.header['num_config_files'] 
        self.config_dict = {}
        for n in range(num_config_files):
            with open(self.infile, 'r') as log_file:
                temp_config_dict = json.loads(log_file.readline())
            self.config_dict.update(temp_config_dict)

    def read_log(self):
        with open(self.infile, 'r') as log_file:
            for line in log_file.readlines():
                yield json.loads(line)
            


if __name__ == '__main__':
    ex_logReader = logReader('logging/dir_log/2019_03_19_1.log')

    pp(ex_logReader.header)
    pp(ex_logReader.config_dict)
    msg_generator = ex_logReader.read_log()

    while 1:
        pp(msg_generator.__next__())

    pdb.set_trace()
