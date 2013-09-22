#shared
source_files = ['src/parallel-primes.c']
env = Environment(CC = 'gcc', CCFLAGS=['-Wall'])

#optimized env
opt = env.Clone()
opt.Append(CCFLAGS = ['-O3'])

#debug env
dbg = env.Clone()
dbg.Append(CCFLAGS = ['-g', '-DDEBUG=1'])

#main program
shell = env.Program('build/parallel-primes', source_files)

#optimized program
o = opt.Object('build/pp-opt', source_files)
optimize = opt.Program(o)

#debug program
d = dbg.Object('build/pp-dbg', source_files)
debug = dbg.Program(d)
