-- Helper functions --

PROJ_ROOT = path.getabsolute('.');

setup = {}
action = {}
action.vs = {}
action.os = {}
setup.nuget = {}
setup.linux = {}

function action.os.type()
    if _ACTION and string.find(_ACTION, 'vs[0-9]*') then
        return 'windows'
    elseif
        _ACTION == 'gmake' or
        _ACTION == 'monodevelop' or
        _ACTION == 'codelite' or
        _ACTION == 'xcode4' then
        return 'linux'
    else
        return 'unknown'
    end
end

function action.vs.toolset()
    local toolset =
        _ACTION == 'vs2015' and '140' or
        _ACTION == 'vs2013' and '120' or
        _ACTION == 'vs2012' and '110' or
        _ACTION == 'vs2010' and '100' or
        _ACTION == 'vs2008' and '90'
    return toolset
end

-- Setup functions --

function setup.nuget.packages()
    local file = assert(io.open('build/studybox_cv/packages.config', 'w'))
    local config =
      [[<?xml version="1.0" encoding="utf-8"?>
        <packages>
            <package id="boost" version="1.60.0.0" targetFramework="native"/>
            <package id="boost_unit_test_framework-vc140" version="1.60.0.0" targetFramework="native"/>
            <package id="opencv3.1" version="1.0" targetFramework="native"/>
            <package id="opencv3.1.redist" version="1.0" targetFramework="native"/>
            <package id="leptonica" version="1.73" targetFramework="native" />
            <package id="leptonica-vc140" version="1.73" targetFramework="native" />
            <package id="tesseract" version="3.04" targetFramework="native" />
            <package id="tesseract-vc140" version="3.04" targetFramework="native" />
        </packages>]]
    file:write(config)
    file:close()
end

function setup.nuget.config()
    local file = assert(io.open('build/nuget.config', 'w'))
    local path = string.gsub(os.getcwd(), '/', '\\')..'\\3rdparty\\'
    local config =
      [[<?xml version="1.0" encoding="utf-8"?>
        <configuration>
        <disabledPackageSources>
            <add key="Microsoft and .NET" value="true" />
        </disabledPackageSources>
        <packageRestore>
            <add key="enabled" value="True" />
            <add key="automatic" value="True" />
        </packageRestore>
        <bindingRedirects>
            <add key="skip" value="False" />
        </bindingRedirects>
        <packageSources>
            <add key="nuget.org" value="https://api.nuget.org/v3/index.json" protocolVersion="3" />
            <add key="studyboxcv" value="]]..path..[[" />
        </packageSources>
        <activePackageSource>
            <add key="nuget.org" value="https://api.nuget.org/v3/index.json" />
        </activePackageSource>
        </configuration>]]
    file:write(config)
    file:close()
end

function setup.linux.opencv()
    if os.execute('sudo stat /usr/local/lib/libopencv_core.so.3.1.0 > /dev/null 2>&1') ~= 0 then
        print('Setting up "OpenCV"')
        local tmp = os.tmpname()
        local file = assert(io.open(tmp, 'w'))
        file:write([[
            #!/bin/bash
            set -e
            apt-get --yes update
            apt-get --yes --force-yes install build-essential cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev
            cd /opt ; wget -O opencv.tar.gz https://github.com/Itseez/opencv/archive/3.1.0.tar.gz
            mkdir opencv ; tar -zxvf opencv.tar.gz -C opencv --strip-components 1
            cd opencv
            mkdir -p 3rdparty/ippicv/downloads/linux-808b791a6eac9ed78d32a7666804320e
            wget -P 3rdparty/ippicv/downloads/linux-808b791a6eac9ed78d32a7666804320e/ https://raw.githubusercontent.com/Itseez/opencv_3rdparty/81a676001ca8075ada498583e4166079e5744668/ippicv/ippicv_linux_20151201.tgz
            for config in debug release
            do
                mkdir $config ; cd $config
                cmake -D CMAKE_BUILD_TYPE=$config -D CMAKE_INSTALL_PREFIX=/usr/local ..
                make ; make install ; cd ..
            done
            ldconfig
        ]])
        file:close()
        os.execute('sudo chmod +x '..tmp..'; sudo '..tmp)
        os.execute('sudo rm -rf /opt/opencv /opt/opencv.tar.gz '..tmp)
    end
end

function setup.linux.boost()
    if os.execute('sudo stat /usr/local/lib/libboost_unit_test_framework.so.1.60.0 > /dev/null 2>&1') ~= 0 then
        print('Setting up "Boost"')
        local tmp = os.tmpname()
        local file = assert(io.open(tmp, 'w'))
        file:write([[
            #!/bin/bash
            set -e
            apt-get --yes update
            apt-get --yes --force-yes install build-essential cmake git p7zip-full
            cd /opt ; wget -O boost.tar.gz https://sourceforge.net/projects/boost/files/boost/1.60.0/boost_1_60_0.tar.gz
            mkdir boost ; tar -zxvf boost.tar.gz -C boost --strip-components 1
            cd boost
            ./bootstrap.sh -with-libraries=test,log,random,thread,locale,regex,filesystem,date_time
            ./b2 install
            ldconfig
        ]])
        file:close()
        os.execute('sudo chmod +x '..tmp..'; sudo '..tmp)
        os.execute('sudo rm -rf /opt/boost /opt/boost.tar.gz '..tmp)
    end
end

function setup.linux.tesseract()
    if os.execute('sudo stat /usr/local/lib/libtesseract.so > /dev/null 2>&1') ~= 0 then
        print('Setting up "Tesseract"')
        local tmp = os.tmpname()
        local file = assert(io.open(tmp, 'w'))
        file:write([[
            #!/bin/bash ; set -e
            apt-get --yes --force-yes --assume-yes install autoconf automake libtool libpng12-dev libjpeg62-dev libtiff5-dev zlib1g-dev
            cd /opt ; wget http://www.leptonica.com/source/leptonica-1.73.tar.gz -O leptonica.tar.gz
            mkdir leptonica ; tar -zxvf leptonica.tar.gz -C leptonica --strip-components 1
            cd leptonica
            ./configure ; make ; make install
            cd .. ; wget https://github.com/tesseract-ocr/tesseract/archive/3.04.01.tar.gz -O tesseract.tar.gz
            mkdir tesseract ; tar -zxvf tesseract.tar.gz -C tesseract --strip-components 1
            cd tesseract
            ./autogen.sh ; ./configure ; make ; make install ; ldconfig
            cd / ; rm -rf /opt/tesseract /opt/leptonica "$0"
        ]])
        file:close()
        os.execute('sudo chmod +x '..tmp..'; sudo '..tmp)
    end
end

-- Premake rules --

workspace 'StudyBox_CV'
    location 'build'
    flags 'FatalWarnings'
    startproject 'StudyBox_CV'
    configurations { 'Debug', 'Release', 'Test' }
    targetdir 'bin/%{cfg.platform}/%{cfg.buildcfg}'

    platforms 'x64'
    architecture 'x64'

    filter 'debug'
        flags 'Symbols'
        defines 'DEBUG'
    filter 'release or test'
        optimize 'On'
        defines 'NDEBUG'

    filter 'action:vs*'
        defines { 'WIN32', 'NOMINMAX', 'NOGDI', 'ABSOLUTE_PATH="'..PROJ_ROOT..'"' }
    filter 'action:not vs*'
        buildoptions '-std=c++0x'
        defines { 'ABSOLUTE_PATH=\\"'..PROJ_ROOT..'\\"' }

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

        filter { 'not test', 'files:**/test/** or files:**maintest.cpp' }
            flags 'ExcludeFromBuild'
        filter { 'test', 'files:**main.cpp' }
            flags 'ExcludeFromBuild'
        filter '*'

        if action.os.type() == 'windows' then
            local toolset = action.vs.toolset()

            filter '*'
                links {
                    'liblept',
                    'libtesseract'
                }
            filter 'release or test'
                links {
                    'opencv_world310',
                }
            filter 'debug'
                links {
                    'opencv_world310d'
                }
            filter 'test'
                links {
                    'boost_unit_test_framework-vc'..toolset..'-mt-1_60'
                }
            filter '*'

            includedirs {
                'build/packages/opencv3.1.1.0/build/native/include',
                'build/packages/boost.1.60.0.0/lib/native/include',
                'build/packages/leptonica.1.73/lib/native/include',
                'build/packages/tesseract.3.04/lib/native/include'
            }

            libdirs {
                'build/packages/opencv3.1.1.0/build/native/lib/%{cfg.platform}/v'..toolset..'/%{cfg.buildcfg == "Test" and "Release" or cfg.buildcfg}',
                'build/packages/boost_unit_test_framework-vc'..toolset..'.1.60.0.0/lib/native/address-model-%{string.sub(cfg.platform, -2)}/lib',
                'build/packages/leptonica-vc'..toolset..'.1.73/lib/native/%{cfg.platform == "Win32" and "x86" or "x64"}/%{cfg.buildcfg == "Test" and "Release" or cfg.buildcfg}',
                'build/packages/tesseract-vc'..toolset..'.3.04/lib/native/%{cfg.platform == "Win32" and "x86" or "x64"}/%{cfg.buildcfg == "Test" and "Release" or cfg.buildcfg}'
            }
            debugenvs {
                'PATH=%PATH%;'..
                '../packages/opencv3.1.redist.1.0/build/native/bin/%{cfg.platform}/v'..toolset..'/%{cfg.buildcfg == "Test" and "Release" or cfg.buildcfg};'..
                '../packages/boost_unit_test_framework-vc'..toolset..'.1.60.0.0/lib/native/address-model-%{string.sub(cfg.platform, -2)}/lib;'..
                '../packages/leptonica-vc'..toolset..'.1.73/lib/native/%{cfg.platform == "Win32" and "x86" or "x64"}/%{cfg.buildcfg == "Test" and "Release" or cfg.buildcfg};'..
                '../packages/tesseract-vc'..toolset..'.3.04/lib/native/%{cfg.platform == "Win32" and "x86" or "x64"}/%{cfg.buildcfg == "Test" and "Release" or cfg.buildcfg}'
            }

            filter 'test'
                postbuildcommands {
                    'xcopy /Y "$(SolutionDir)packages\\opencv3.1.redist.1.0\\build\\native\\bin\\%{cfg.platform}\\v'..toolset..'\\%{cfg.buildcfg == "Test" and "Release" or cfg.buildcfg}\\opencv_world310.dll" "$(TargetDir)"',
                    'xcopy /Y "$(SolutionDir)packages\\boost_unit_test_framework-vc'..toolset..'.1.60.0.0\\lib\\native\\address-model-%{string.sub(cfg.platform, -2)}\\lib\\boost_unit_test_framework-vc'..toolset..'-mt-1_60.dll" "$(TargetDir)"',
                    'xcopy /Y "$(SolutionDir)packages\\leptonica-vc'..toolset..'.1.73\\lib\\native\\%{cfg.platform == "Win32" and "x86" or "x64"}\\%{cfg.buildcfg == "Test" and "Release" or cfg.buildcfg}\\liblept.dll" "$(TargetDir)"',
                    'xcopy /Y "$(SolutionDir)packages\\tesseract-vc'..toolset..'.3.04\\lib\\native\\%{cfg.platform == "Win32" and "x86" or "x64"}\\%{cfg.buildcfg == "Test" and "Release" or cfg.buildcfg}\\libtesseract.dll" "$(TargetDir)"',
                    '"$(TargetDir)\\$(TargetName).exe" --result_code=no --report_level=short'
                }

        elseif action.os.type() == 'linux' then
            includedirs {
                'usr/local/include'
            }
            libdirs {
                'usr/local/lib'
            }
            links {
                'ssl',
                'crypto',
                'pthread',
                'tesseract',
                'opencv_core',
                'boost_system',
                'opencv_imgproc',
                'opencv_highgui',
                'opencv_imgcodecs'
            }
            filter 'test'
                links {
                    'boost_unit_test_framework'
                }
        end

-- Premake overrides --

premake.override(premake.main, 'preAction', function(base)
    base()

    io.stdout:setvbuf('no')

    if _OPTIONS['clean'] then
        print('Cleaning bin and build directories...')
        os.rmdir('./build')
        os.rmdir('./bin')
    end
end)

premake.override(premake.main, 'postAction', function(base)
    base()

    local builds = {}
    if _OPTIONS['build'] == 'all' then
        builds = { 'release', 'debug', 'test' }
    elseif _OPTIONS['build'] == 'app' then
        builds = { 'release' }
    elseif _OPTIONS['build'] == 'test' or _OPTIONS['test'] then
        builds = { 'test' }
    end

    local runs = nil
    if _OPTIONS['run'] == 'app' then
        runs = 'Release'
    elseif _OPTIONS['run'] == 'test' or _OPTIONS['test'] then
        runs = 'Test'
    end

    if action.os.type() == 'windows' then
        setup.nuget.config()
        setup.nuget.packages()
    elseif action.os.type() == 'linux' then
        setup.linux.boost()
        setup.linux.opencv()
        setup.linux.tesseract()
    end

    if next(builds) then
        if action.os.type() == 'windows' then
            local file = io.open('build/nuget.exe')
            if file == nil then
                print('Downloading `nuget.exe`...')
                os.execute('powershell wget dist.nuget.org/win-x86-commandline/latest/nuget.exe -OutFile build/nuget.exe')
                print('Resolving dependancies...')
                os.execute('cd build & call "%VS'..action.vs.toolset()..'COMNTOOLS%VsMSBuildCmd.bat" & nuget restore /Verbosity quiet')
            else
                file:close()
            end
        end

        for k,v in pairs(builds) do
            print()
            if action.os.type() == 'linux' then
                os.execute('cd build ; make config='..v..'_x64')
            elseif action.os.type() == 'windows' then
                print('Building `'..v..'`...')
                os.execute('cd build & call "%VS'..action.vs.toolset()..'COMNTOOLS%VsMSBuildCmd.bat" & msbuild /nologo /v:quiet /clp:ErrorsOnly /p:Configuration='..v)
            end
        end
    end

    if runs then
        print('\nRunning `'..runs..'`...\n')
        os.execute('"'..PROJ_ROOT..'/bin/x64/'..runs..'/StudyBox_CV"') 
    end
end)

-- Extra options --

newoption {
    trigger = 'test',
    description = 'Build and execute tests'
}

newoption {
    trigger = 'build',
    value = 'VALUE',
    description = 'Build configuration',
    allowed = {
        { 'all', 'All' },
        { 'app', 'Application' },
        { 'test', 'Test' }
    }
}

newoption {
    trigger = 'clean',
    description = 'Remove build and bin directories'
}

newoption {
    trigger = 'run',
    value = 'default',
    description = 'Runs specified target',
    allowed = {
        { 'app', 'Application' },
        { 'test', 'Test' }
    }
}

-- Extra actions --

newaction {
    trigger = 'clean',
    description = 'Removes build and bin directories',
    execute = function()
        os.rmdir('./build')
        os.rmdir('./bin')
        print('Done')
    end
}
