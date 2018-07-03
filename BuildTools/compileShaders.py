import sys
import subprocess
import glob

if len(sys.argv) != 2:
    print "Wrong number of arguments!"
    sys.exit()

vShaders = glob.glob(sys.argv[1] + "\\*.vert")
fShaders = glob.glob(sys.argv[1] + "\\*.frag")

print("vertex shaders" + str(vShaders))
print("fragment shaders" + str(fShaders))

for v in vShaders:
    outputName = str(v).split(".vert")[0] + "_vert.spv"
    subprocess.call(["glslangValidator.exe", "-V", str(v), "-o", outputName])
    print "output: " + outputName

for f in fShaders:
    outputName = str(f).split(".frag")[0] + "_frag.spv"
    subprocess.call(["glslangValidator.exe", "-V", str(f), "-o", outputName])
    print "output: " + outputName

