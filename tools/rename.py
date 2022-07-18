import os

def jp2chn(source):
    return source.encode('gbk').decode('shift-jis')

for root, dirs, files in os.walk('.', topdown=False):
    print(root, dirs, files)
    for name in files:
        oldname = os.path.join(root, name)
        newname = os.path.join(root, jp2chn(name))
        print('{0} -> {1}'.format(oldname, newname))
        os.rename(oldname, newname)
    for name in dirs:
        oldname = os.path.join(root, name)
        newname = os.path.join(root, jp2chn(name))
        print('{0} -> {1}'.format(oldname, newname))
        os.rename(oldname, newname)