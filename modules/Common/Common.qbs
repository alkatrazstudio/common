/****************************************************************************}
{ Common.qbs - common project settings                                       }
{                                                                            }
{ Copyright (c) 2018 Alexey Parfenov <zxed@alkatrazstudio.net>               }
{                                                                            }
{ This library is free software: you can redistribute it and/or modify it    }
{ under the terms of the GNU General Public License as published by          }
{ the Free Software Foundation, either version 3 of the License,             }
{ or (at your option) any later version.                                     }
{                                                                            }
{ This library is distributed in the hope that it will be useful,            }
{ but WITHOUT ANY WARRANTY; without even the implied warranty of             }
{ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU           }
{ General Public License for more details: https://gnu.org/licenses/gpl.html }
{****************************************************************************/

import qbs.File
import qbs.FileInfo
import qbs.TextFile
import qbs.Environment

Module {
    readonly property string installDir: FileInfo.joinPaths(qbs.installRoot, qbs.installPrefix)
    readonly property bool realInstall: product.sourceDirectory !== installDir
    readonly property string srcPath: FileInfo.relativePath(path, product.sourceDirectory)
    readonly property string srcPrefix: srcPath+'/'
    property stringList qmlIncludes: []
    property stringList qmlImportPaths: []

    property string appName: product.name
    property string appTitle: appName
    property string orgTitle: 'Alkatraz Studio'
    property string orgDomain: 'alkatrazstudio.net'
    property string orgReverseDomain: orgDomain.split('.').reverse().join('.')
    property string appReverseDomain: orgReverseDomain+'.'+appName.replace(/-/g, '')
    readonly property bool isLib: product.type.indexOf('dynamiclibrary') != -1
    property bool installLibLinks: true
    property string osxIconFilename: 'icons/app.icns'

    readonly property bool isLinux: qbs.targetPlatform === 'linux'
    readonly property bool isWindows: qbs.targetPlatform === 'windows'
    readonly property bool isOSX: qbs.targetPlatform === 'macos'

    readonly property stringList libraryPaths: {
        var paths = []
        if(product.Qt)
        {
            paths.push(product.Qt.core.libPath)
            if(Common.isLinux)
                paths.push(product.Qt.core.libPath+'/'+qbs.architecture+'-linux-gnu')
            else if(Common.isWindows)
                paths.push(Environment.getEnv('MINGW_PREFIX')+'/bin')
        }
        return paths
    }

    readonly property string libExt: {
        if(isLinux)
            return 'so'
        if(isWindows)
            return 'dll'
        if(isOSX)
            return 'dylib'
        return ''
    }

    Group {
        name: 'TODO'
        files: [
            'TODO.TXT'
        ]
        .filter(function(f){return File.exists(product.sourceDirectory+'/'+f)})
        .map(function(f){return Common.srcPath+'/'+f})
    }

    Depends {name: 'cpp'}
    Depends {name: 'Version'}

    cpp.cxxLanguageVersion: 'c++17'
    cpp.linkerFlags: {
        var flags = []
        if(Common.isLinux)
        {
            flags.push('-fuse-ld=gold')
            flags.push('-L/usr/local/lib')
        }
        return flags
    }

    cpp.libraryPaths: libraryPaths

    cpp.includePaths: {
        var paths = []
        if(product.Qt)
            paths.push(product.Qt.core.incPath)
        paths.push(product.buildDirectory+'/generated/common/include')
        return paths
    }

    Depends {
        name: 'bundle'
        condition: Common.isOSX
    }

    Properties {
        condition: Common.isOSX
        bundle.identifier: {
            return appReverseDomain
        }
    }

    Probe {
        id: osxIconProbe
        condition: Common.isOSX
        readonly property var srcDir: project.sourceDirectory
        readonly property string osxIconFilename: Common.osxIconFilename
        property bool iconFileExists: false
        property string iconFilename: ""
        configure: {
            iconFilename = srcDir + '/' + osxIconFilename
            iconFileExists = File.exists(iconFilename)
            found = iconFileExists
        }
    }
    // need these props because Probes can't be used inside Groups
    readonly property bool osxIconFileExists: osxIconProbe.iconFileExists
    readonly property string osxIconFullFilename: osxIconProbe.iconFilename

    Group {
        name: 'OSX icon'
        condition: Common.isOSX && Common.osxIconFileExists
        files: [
            Common.osxIconFullFilename
        ]
        qbs.install: Common.realInstall
    }

    // can't put this in the above Group, because it will be ignored
    Properties {
        condition: Common.isOSX && Common.osxIconFileExists
        bundle.infoPlist: ({
            CFBundleIconFile: FileInfo.fileName(Common.osxIconFullFilename)
        })
    }

    Rule {
        multiplex: true
        Artifact {
            filePath: 'generated/common/include/common_defines.h'
            fileTags: ['hpp']
        }
        prepare: {
            var cmd = new JavaScriptCommand()
            cmd.description = 'generating ' + output.fileName
            cmd.sourceCode = function() {
                var f = new TextFile(output.filePath, TextFile.WriteOnly)
                f.writeLine('// This file is autogenerated each rebuild.')
                f.writeLine('#pragma once')
                f.writeLine('constexpr const unsigned int VER_MAJ = '+product.Version.major+';')
                f.writeLine('constexpr const unsigned int VER_MIN = '+product.Version.minor+';')
                f.writeLine('constexpr const unsigned int VER_PAT = '+product.Version.patch+';')
                f.writeLine('constexpr const char* const APP_NAME = "'+product.Common.appName+'";')
                f.writeLine('constexpr const char* const APP_TITLE = "'+product.Common.appTitle+'";')
                f.writeLine('constexpr const char* const APP_ORG = "'+product.Common.orgTitle+'";')
                f.writeLine('constexpr const char* const APP_ORG_DOMAIN = "'+product.Common.orgDomain+'";')
                f.writeLine('constexpr const char* const SOURCE_ROOT = "'+product.sourceDirectory+'";')
                f.writeLine('constexpr const unsigned long long BUILD_TS = '+Math.round(Date.now() / 1000)+';')
                f.close()
            }
            return cmd
        }
    }

    Properties {
        condition: qbs.buildVariant === 'release'
        cpp.debugInformation: false
        cpp.enableDebugCode: false
        cpp.cxxFlags: outer.concat('-fvisibility=hidden')
        cpp.linkerFlags: {
            var flags = outer
            if(!Common.isOSX)
                flags = flags.concat('-s')
            if(Common.realInstall && cpp.rpathOrigin)
                flags = flags.concat('-rpath').concat(cpp.rpathOrigin+'/lib')
            return flags
        }
        cpp.useRPaths: !Common.realInstall
        cpp.enableReproducibleBuilds: true
    }

    Properties {
        condition: qbs.buildVariant === 'debug'
        cpp.debugInformation: true
        cpp.enableDebugCode: true
        cpp.cxxFlags: outer.concat('-ggdb3').concat('-fvisibility=hidden')
    }

    Properties {
        condition: Common.isWindows
        cpp.defines: outer.concat([
            'WIN32_LEAN_AND_MEAN',
            '_WIN32_IE=_WIN32_IE_WIN10',
            'UNICODE',
            '_UNICODE'
        ])
    }

    Group {
        name: 'Binary'
        fileTagsFilter: {
            var tags = []
            if(Common.isLib)
            {
                tags.push('dynamiclibrary')
            }
            else
            {
                tags.push('bundle.content')
                if(!Common.isOSX)
                    tags.push('application')
            }
            tags = product.type.filter(function(tag){return tags.indexOf(tag) != -1})
            if(Common.isLib && Common.installLibLinks)
                tags.push('dynamiclibrary_symlink')
            return tags
        }
        qbs.install: true
    }

    Group {
        name: 'Translations'
        fileTags: ['ts-common']
        prefix: Common.srcPrefix
        files: ['translations/*.ts']
    }

    additionalProductTypes: ['qm-common']

    Rule {
        inputs: ['ts-common']

        Artifact {
            filePath: FileInfo.joinPaths('translations', input.baseName+'.qm')
            fileTags: ['qm-common']
            qbs.install: true
            qbs.installDir: 'translations'
        }

        prepare: {
            var args = [
                '-silent',
                '-compress',
                '-nounfinished',
                '-removeidentical',
                '-qm',
                output.filePath,
                input.filePath
            ]
            var exe = FileInfo.joinPaths(product.Qt.core.binPath, 'lrelease')
            var cmd = new Command(exe, args)

            cmd.description = 'Creating '+output.fileName
            cmd.highlight = 'filegen'
            return cmd
        }
    }
}
