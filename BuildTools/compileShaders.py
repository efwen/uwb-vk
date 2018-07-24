import sys
import subprocess
import glob
import codecs

if len(sys.argv) != 2:
    print("Wrong number of arguments!")
    sys.exit()

vShaders = glob.glob(sys.argv[1] + "\\*.vert")
fShaders = glob.glob(sys.argv[1] + "\\*.frag")
tcShaders = glob.glob(sys.argv[1] + "\\*.tesc")
teShaders = glob.glob(sys.argv[1] + "\\*.tese")
gShaders = glob.glob(sys.argv[1] + "\\*.geom")

def process(shaderList, ext):
    print("processing " + ext + " shaders:")
    for shader in shaderList:
        outputName = str(shader).split("." + ext)[0] + "_" + ext + ".spv"
        subprocess.call(["glslangValidator.exe", "-V", str(shader), "-o", outputName])
        print("output: " + outputName)
    return

process(vShaders, "vert")
process(fShaders, "frag")
process(tcShaders, "tesc")
process(teShaders, "tese")
process(gShaders, "geom")
