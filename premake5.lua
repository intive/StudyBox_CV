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
        defines 'WIN32'
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

        if _ACTION and string.find(_ACTION, 'vs*') then
            local toolset = _ACTION == 'vs2015' and '140' or
                            _ACTION == 'vs2013' and '120' or
                            _ACTION == 'vs2012' and '110' or
                            _ACTION == 'vs2010' and '100' or
                            _ACTION == 'vs2008' and '90'

            includedirs {
                'build/packages/opencv3.1.1.0/build/native/include',
                'build/packages/boost.1.60.0.0/lib/native/include',
                'build/packages/wastorage.v'..toolset..'.2.2.0/build/native/include',
                'build/packages/cpprestsdk.v'..toolset..'.windesktop.msvcstl.dyn.rt-dyn.2.7.0/build/native/include'
            }
            libdirs {
                'build/packages/opencv3.1.1.0/build/native/lib/%{cfg.platform}/v'..toolset..'/%{cfg.buildcfg}',
                'build/packages/boost_unit_test_framework-vc'..toolset..'.1.60.0.0/lib/native/address-model-%{string.sub(cfg.platform, -2)}/lib',
                'build/packages/wastorage.v'..toolset..'.2.2.0/lib/native/v'..toolset..'/%{cfg.platform}/%{cfg.buildcfg}',
                'build/packages/cpprestsdk.v'..toolset..'.windesktop.msvcstl.dyn.rt-dyn.2.7.0/lib/native/v'..toolset..'/windesktop/msvcstl/dyn/rt-dyn/%{cfg.platform == "Win32" and "x86" or "x64"}/%{cfg.buildcfg}'
            }
            debugenvs {
                'PATH=%PATH%;'..
                '../packages/opencv3.1.redist.1.0/build/native/bin/%{cfg.platform}/v'..toolset..'/%{cfg.buildcfg};'..
                '../packages/boost_unit_test_framework-vc'..toolset..'.1.60.0.0/lib/native/address-model-%{string.sub(cfg.platform, -2)}/lib;'..
                '../packages/wastorage.v'..toolset..'.2.2.0/lib/native/v'..toolset..'/%{cfg.platform}/%{cfg.buildcfg};'..
                '../packages/cpprestsdk.v'..toolset..'.windesktop.msvcstl.dyn.rt-dyn.2.7.0/lib/native/v'..toolset..'/windesktop/msvcstl/dyn/rt-dyn/%{cfg.platform == "Win32" and "x86" or "x64"}/%{cfg.buildcfg}'
            }

            filter '*'
                links {
                    'wastorage'
                }
            filter 'release'
                links {
                    'cpprest140_2_7',
                    'opencv_world310',
                    'boost_unit_test_framework-vc'..toolset..'-mt-1_60'
                }
            filter 'debug'
                links {
                    'cpprest140d_2_7',
                    'opencv_world310d',
                    'boost_unit_test_framework-vc' .. toolset .. '-mt-gd-1_60'
                }

            local file = assert(io.open('build/studybox_cv/packages.config', 'w'))
            local config =
              [[<?xml version="1.0" encoding="utf-8"?>
                <packages>
                <package id="boost" version="1.60.0.0" targetFramework="native"/>
                <package id="boost_unit_test_framework-vc]]..toolset..[[" version="1.60.0.0" targetFramework="native"/>
                <package id="opencv3.1" version="1.0" targetFramework="native"/>
                <package id="opencv3.1.redist" version="1.0" targetFramework="native"/>
                <package id="wastorage.v]]..toolset..[[" version="2.2.0" targetFramework="native"/>
                <package id="cpprestsdk.v]]..toolset..[[.windesktop.msvcstl.dyn.rt-dyn" version="2.7.0" targetFramework="native"/>
                </packages>]]
            file:write(config)
            file:close()
        end

newaction {
   trigger = 'clean',
   description = 'Removes build and bin directories',
   execute = function()
      os.rmdir('./build')
      os.rmdir('./bin')
      print('Done')
   end
}
