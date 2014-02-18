
env = Environment(
	LIBPATH = ["."]
)

env.MergeFlags("--std=c++11 -g")

env.StaticLibrary("nbt", "nbt.cpp")

env.Program("test", "test.cpp",
	LIBS = "nbt"
)

