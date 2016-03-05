workspace 'StudyBox_CV'
    location 'build'
    flags 'FatalWarnings'
    startproject 'StudyBox_CV'
    configurations { 'Debug', 'Release' }
    targetdir 'bin/%{cfg.platform}/%{cfg.buildcfg}'

    filter 'debug'
        flags 'Symbols'
        defines 'DEBUG'
    filter 'release'
        optimize 'On'
        defines 'NDEBUG'

    filter 'action:vs*'
        platforms { 'Win32', 'x64' }
    filter 'action:not vs*'
        platforms { 'x32', 'x64' }
        buildoptions '-std=c++0x'

    project 'StudyBox_CV'
        language 'C++'
        kind 'ConsoleApp'
        location 'build/studybox_cv'

        files {
            'source/**'
        }

newaction {
   trigger = 'clean',
   description = 'Removes build and bin directories',
   execute = function()
      os.rmdir('./build')
      os.rmdir('./bin')
      print('Done')
   end
}
