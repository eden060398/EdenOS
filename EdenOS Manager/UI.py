import sys

class Progress:
    def __init__(self, percent=0):
        self.percent = percent
        done = 'X' * (percent / 2)
        not_done = ' ' * (50 - percent / 2)
        sys.stdout.write('|{done}{not_done}| {percent}%'.format(done=done, not_done=not_done,percent=percent))
        sys.stdout.flush()

    def update(self, percent):
        if percent != self.percent:
            self.percent = percent
            done = 'X' * (percent / 2)
            not_done = ' ' * (50 - percent / 2)
            sys.stdout.write('\r|{done}{not_done}| {percent}%'.format(done=done, not_done=not_done, percent=percent))
            sys.stdout.flush()

    @staticmethod
    def done():
        sys.stdout.write('\n')
        sys.stdout.flush()
