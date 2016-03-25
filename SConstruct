
vars = Variables(None, ARGUMENTS)

vars.AddVariables(
	BoolVariable("debug", "Make a debug build", False)
)

env = Environment(
	variables = vars,
	LIBPATH = ["bin"]
)

Help(vars.GenerateHelpText(env))

env.MergeFlags("--std=c++11 -Wall -Wextra -Wpedantic")
if env["debug"]:
	env.MergeFlags("-g")
else:
	env.MergeFlags("-O3")

env.StaticLibrary("bin/nbt", Split("""
	nbt.cpp
	serialization.cpp
	compression.cpp
"""))

env.Program("bin/test", "test.cpp",
	LIBS = ["nbt", "z"]
)

