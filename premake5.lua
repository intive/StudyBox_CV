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
		excludes {
		    '**/example.cpp'
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

        if _ACTION == 'gmake' or
           _ACTION == 'monodevelop' or
           _ACTION == 'codelite' or
           _ACTION == 'xcode4' then
            includedirs {
                'usr/local/include'
            }
            libdirs {
                'usr/local/lib'
            }
            links {
                'ssl',
                'crypto',
                'cpprest',
                'azurestorage',
                'opencv_core',
                'opencv_imgproc',
                'opencv_highgui',
                'opencv_imgcodecs',
                'boost_system',
                'boost_unit_test_framework',
                'pthread'
            }

            if os.execute('sudo stat /usr/local/lib/libopencv_core.so.3.1.0 > /dev/null 2>&1') ~= 0 then
                print('Setting up "OpenCV 3.1.0"')
                local tmp = os.tmpname()
                local file = assert(io.open(tmp, 'w'))
                file:write([[
                    #!/bin/bash ; set -e
                    apt-get --yes --force-yes install build-essential cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev
                    cd /opt ; git clone https://github.com/Itseez/opencv.git
                    cd opencv ; git checkout 3.1.0
                    for config in debug release
                    do
                        mkdir $config ; cd $config
                        cmake -D CMAKE_BUILD_TYPE=$config -D CMAKE_INSTALL_PREFIX=/usr/local ..
                        make ; make install ; cd ..
                    done
                    ldconfig
                    cd / ; rm -rf /opt/opencv "$0"
                ]])
                file:close()
                os.execute('sudo chmod +x '..tmp..'; sudo '..tmp)
            end

            if os.execute('sudo stat /usr/local/lib/libboost_unit_test_framework.so.1.60.0 > /dev/null 2>&1') ~= 0 then
                print('Setting up "Boost Unit Test Framework 1.60"')
                local tmp = os.tmpname()
                local file = assert(io.open(tmp, 'w'))
                file:write([[
                    #!/bin/bash ; set -e
                    apt-get --yes --force-yes install p7zip-full
                    cd /opt ; wget -O boost_1_60_0.7z "http://downloads.sourceforge.net/project/boost/boost/1.60.0/boost_1_60_0.7z?r=https%3A%2F%2Fsourceforge.net%2Fprojects%2Fboost%2Ffiles%2Fboost%2F1.60.0%2F&ts=1457189367&use_mirror=netcologne boost_1_60_0.7z"
                    7z x boost_1_60_0.7z ; cd boost_1_60_0
                    ./bootstrap.sh -with-libraries=test,log,random,thread,locale,regex,filesystem,date_time
                    ./b2 install
                    ldconfig
                    cd / ; rm -rf /opt/boost_1_60_0 /opt/boost_1_60_0.7z "$0"
                ]])
                file:close()
                os.execute('sudo chmod +x '..tmp..'; sudo '..tmp)
            end

            if os.execute('sudo stat /usr/local/lib/libcpprest.so > /dev/null 2>&1') ~= 0 then
                print('Setting up "Azure Storage CPP"')
                local tmp = os.tmpname()
                local file = assert(io.open(tmp, 'w'))
                file:write([[
                    #!/bin/bash ; set -e
                    apt-get --yes --force-yes install g++-4.8 g++ git make libssl-dev cmake libxml++2.6-dev libxml++2.6-doc uuid-dev
                    cd /opt ; git clone https://github.com/Microsoft/cpprestsdk.git casablanca
                    cd casablanca/Release
                    for config in debug release
                    do
                        mkdir build_$config ; cd build_$config
                        cmake -D CMAKE_BUILD_TYPE=$config -D CMAKE_INSTALL_PREFIX=/usr/local ..
                        make ; make install ; cd ..
                    done
                    ldconfig
                    cd /opt ; git clone https://github.com/Azure/azure-storage-cpp.git
                    cd azure-storage-cpp/Microsoft.WindowsAzure.Storage
                    for config in debug release
                    do
                        mkdir build_$config ; cd build_$config
                        cmake -D CMAKE_BUILD_TYPE=$config -D CMAKE_INSTALL_PREFIX=/usr/local ..
                        make ; make install
                        mv -v Binaries/* /usr/local/lib/ ; cd ..
                    done
                    cp -r includes/was /usr/local/include/
                    cp -r includes/wascore /usr/local/include/
                    ldconfig
                    cd / ; rm -rf /opt/casablanca /opt/azure-storage-cpp "$0"
                ]])
                file:close()
                os.execute('sudo chmod +x '..tmp..'; sudo '..tmp)
            end
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
