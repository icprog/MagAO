class bcolors:
    HEADER = '\033[95m'
    OKGREEN = '\033[92m'
    WARNING = '\033[94m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    RESET = '\033[0;0m'

    def disable(self):
        self.HEADER = ''
        self.OKBLUE = ''
        self.OKGREEN = ''
        self.WARNING = ''
        self.FAIL = ''
        self.ENDC = ''

