##mysetup.py

from distutils.core import setup
import py2exe
setup(
    version = '1.0.0.1',
    description = 'tkinter for platfrom test',
    author = 'xinwei',
    windows = ['PLTTest.py'],
    data_files = [('', ['myicon.ico', 'para.ini', 'Testlist.db'])],
    
    )
