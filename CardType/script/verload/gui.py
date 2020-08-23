__author__ = 'xuqiang'

# coding = utf-8

from tkinter import *
from tkinter import ttk
from tkinter.scrolledtext import *
from tkinter import tix


class Gui:
    def __init__(self):
        self.top = tix.Tk(className='版本升级')
        self.top.geometry('=300x400+500+200')
        # self.top.attributes('-alpha',1,'-topmost', 1)
        self.top.iconbitmap('myicon.ico')
        # anchor=NW
        self.frame_basic = LabelFrame(self.top, relief=GROOVE, borderwidth=2, text='配置信息', foreground='blue')
        self.frame_basic.pack(anchor=NW, padx=5, pady=5)

        self.frame_config = Frame(self.frame_basic, relief=FLAT)
        self.frame_config.pack(padx=5, pady=5)

        self.frame_directory = Frame(self.frame_basic, relief=FLAT)
        self.frame_directory.pack(padx=5, pady=5)

        self.frame_update = Frame(self.frame_basic, relief=FLAT)
        self.frame_update.pack(padx=5, pady=5, side=LEFT)

        # frame_config
        self.dst_addr = tix.LabelEntry(self.frame_config)
        self.dst_addr.pack(side=LEFT)
        self.dst_addr.label['text'] = '目的地址:'
        self.dst_addr.entry['width'] = 20

        self.null_config = Label(self.frame_config, width=8)
        self.null_config.pack(after=self.dst_addr, side=LEFT)

        self.button_addr_set = ttk.Button(self.frame_config, text='设置', width=5)
        self.button_addr_set.pack(after=self.null_config, side=LEFT)

        # frame_dirctory
        self.file_directory = tix.LabelEntry(self.frame_directory)
        self.file_directory.pack(side=LEFT)
        self.file_directory.label['text'] = '版本路径:'
        self.file_directory.entry['width'] = 20

        self.null_directory = Label(self.frame_directory, width=8)
        self.null_directory.pack(after=self.file_directory, side=LEFT)

        self.button_addr_set = ttk.Button(self.frame_directory, text='浏览', width=5)
        self.button_addr_set.pack(after=self.null_directory, side=LEFT)

        # frame_update
        self.processbar_update = ttk.Progressbar(self.frame_update, length=160)
        self.processbar_update.pack(side=LEFT, padx=20)

        self.null_update = Label(self.frame_update, width=5)
        self.null_update.pack(side=LEFT)

        self.button_update = ttk.Button(self.frame_update, text='升级', width=5)
        self.button_update.pack(side=LEFT, pady=5)

        self.frame_log = LabelFrame(self.top, relief=GROOVE, borderwidth=2, text='日志信息', foreground='blue')
        self.frame_log.pack(side=LEFT, padx=5)

        self.msg = ScrolledText(self.frame_log, width=47, height=17)
        self.msg.pack(side=LEFT, padx=5, pady=5)


class DirectoryList(tix.TixSubWidget):
    def __init__(self):
        tix.TixSubWidget.__init__(self, master=None, name='DIR')
        self.frame = Frame(master)
        self.directory_tree = tix.DirTree(self.frame)
        self.directory_tree.pack()


def test():
    # Gui()
    DirectoryList()
    mainloop()
if __name__ == '__main__':
    test()