#!/usr/bin/env python
#
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import fnmatch
import optparse
import os
import shutil
import re
import sys
import textwrap

from util import build_utils
from util import md5_check

import jar

sys.path.append(build_utils.COLORAMA_ROOT)
import colorama


def ColorJavacOutput(output):
  fileline_prefix = r'(?P<fileline>(?P<file>[-.\w/\\]+.java):(?P<line>[0-9]+):)'
  warning_re = re.compile(
      fileline_prefix + r'(?P<full_message> warning: (?P<message>.*))$')
  error_re = re.compile(
      fileline_prefix + r'(?P<full_message> (?P<message>.*))$')
  marker_re = re.compile(r'\s*(?P<marker>\^)\s*$')

  warning_color = ['full_message', colorama.Fore.YELLOW + colorama.Style.DIM]
  error_color = ['full_message', colorama.Fore.MAGENTA + colorama.Style.BRIGHT]
  marker_color = ['marker',  colorama.Fore.BLUE + colorama.Style.BRIGHT]

  def Colorize(line, regex, color):
    match = regex.match(line)
    start = match.start(color[0])
    end = match.end(color[0])
    return (line[:start]
            + color[1] + line[start:end]
            + colorama.Fore.RESET + colorama.Style.RESET_ALL
            + line[end:])

  def ApplyColor(line):
    if warning_re.match(line):
      line = Colorize(line, warning_re, warning_color)
    elif error_re.match(line):
      line = Colorize(line, error_re, error_color)
    elif marker_re.match(line):
      line = Colorize(line, marker_re, marker_color)
    return line

  return '\n'.join(map(ApplyColor, output.split('\n')))


def DoJavac(
    classpath, classes_dir, chromium_code, java_files):
  """Runs javac.

  Builds |java_files| with the provided |classpath| and puts the generated
  .class files into |classes_dir|. If |chromium_code| is true, extra lint
  checking will be enabled.
  """

  jar_inputs = []
  for path in classpath:
    if os.path.exists(path + '.TOC'):
      jar_inputs.append(path + '.TOC')
    else:
      jar_inputs.append(path)

  javac_args = [
      '-g',
      # Chromium only allows UTF8 source files.  Being explicit avoids
      # javac pulling a default encoding from the user's environment.
      '-encoding', 'UTF-8',
      '-source', '1.7',
      '-target', '1.7',
      '-classpath', ':'.join(classpath),
      '-d', classes_dir]
  if chromium_code:
    javac_args.extend(['-Xlint:unchecked', '-Xlint:deprecation'])
  else:
    # XDignore.symbol.file makes javac compile against rt.jar instead of
    # ct.sym. This means that using a java internal package/class will not
    # trigger a compile warning or error.
    javac_args.extend(['-XDignore.symbol.file'])

  javac_cmd = ['javac'] + javac_args + java_files

  def Compile():
    build_utils.CheckOutput(
        javac_cmd,
        print_stdout=chromium_code,
        stderr_filter=ColorJavacOutput)

  record_path = os.path.join(classes_dir, 'javac.md5.stamp')
  md5_check.CallAndRecordIfStale(
      Compile,
      record_path=record_path,
      input_paths=java_files + jar_inputs,
      input_strings=javac_cmd)


_MAX_MANIFEST_LINE_LEN = 72


def CreateManifest(manifest_path, classpath, main_class=None):
  """Creates a manifest file with the given parameters.

  This generates a manifest file that compiles with the spec found at
  http://docs.oracle.com/javase/7/docs/technotes/guides/jar/jar.html#JAR_Manifest

  Args:
    manifest_path: The path to the manifest file that should be created.
    classpath: The JAR files that should be listed on the manifest file's
      classpath.
    main_class: If present, the class containing the main() function.

  """
  output = ['Manifest-Version: 1.0']
  if main_class:
    output.append('Main-Class: %s' % main_class)
  if classpath:
    sanitized_paths = []
    for path in classpath:
      sanitized_paths.append(os.path.basename(path.strip('"')))
    output.append('Class-Path: %s' % ' '.join(sanitized_paths))
  output.append('Created-By: ')
  output.append('')

  wrapper = textwrap.TextWrapper(break_long_words=True,
                                 drop_whitespace=False,
                                 subsequent_indent=' ',
                                 width=_MAX_MANIFEST_LINE_LEN - 2)
  output = '\r\n'.join(w for l in output for w in wrapper.wrap(l))

  with open(manifest_path, 'w') as f:
    f.write(output)


def main(argv):
  colorama.init()

  argv = build_utils.ExpandFileArgs(argv)

  parser = optparse.OptionParser()
  build_utils.AddDepfileOption(parser)

  parser.add_option(
      '--src-gendirs',
      help='Directories containing generated java files.')
  parser.add_option(
      '--java-srcjars',
      action='append',
      default=[],
      help='List of srcjars to include in compilation.')
  parser.add_option(
      '--classpath',
      action='append',
      help='Classpath for javac. If this is specified multiple times, they '
      'will all be appended to construct the classpath.')
  parser.add_option(
      '--javac-includes',
      help='A list of file patterns. If provided, only java files that match'
      'one of the patterns will be compiled.')
  parser.add_option(
      '--jar-excluded-classes',
      default='',
      help='List of .class file patterns to exclude from the jar.')

  parser.add_option(
      '--chromium-code',
      type='int',
      help='Whether code being compiled should be built with stricter '
      'warnings for chromium code.')

  parser.add_option(
      '--classes-dir',
      help='Directory for compiled .class files.')
  parser.add_option('--jar-path', help='Jar output path.')
  parser.add_option(
      '--main-class',
      help='The class containing the main method.')

  parser.add_option('--stamp', help='Path to touch on success.')

  options, args = parser.parse_args(argv)

  if options.main_class and not options.jar_path:
    parser.error('--main-class requires --jar-path')

  classpath = []
  for arg in options.classpath:
    classpath += build_utils.ParseGypList(arg)

  java_srcjars = []
  for arg in options.java_srcjars:
    java_srcjars += build_utils.ParseGypList(arg)

  java_files = args
  if options.src_gendirs:
    src_gendirs = build_utils.ParseGypList(options.src_gendirs)
    java_files += build_utils.FindInDirectories(src_gendirs, '*.java')

  input_files = classpath + java_srcjars + java_files
  with build_utils.TempDir() as temp_dir:
    classes_dir = os.path.join(temp_dir, 'classes')
    os.makedirs(classes_dir)
    if java_srcjars:
      java_dir = os.path.join(temp_dir, 'java')
      os.makedirs(java_dir)
      for srcjar in java_srcjars:
        build_utils.ExtractAll(srcjar, path=java_dir, pattern='*.java')
      java_files += build_utils.FindInDirectory(java_dir, '*.java')

    if options.javac_includes:
      javac_includes = build_utils.ParseGypList(options.javac_includes)
      filtered_java_files = []
      for f in java_files:
        for include in javac_includes:
          if fnmatch.fnmatch(f, include):
            filtered_java_files.append(f)
            break
      java_files = filtered_java_files

    DoJavac(
        classpath,
        classes_dir,
        options.chromium_code,
        java_files)

    if options.jar_path:
      if options.main_class:
        manifest_file = os.path.join(temp_dir, 'manifest')
        CreateManifest(manifest_file, classpath,
                       options.main_class)
      else:
        manifest_file = None
      jar.JarDirectory(classes_dir,
                       build_utils.ParseGypList(options.jar_excluded_classes),
                       options.jar_path,
                       manifest_file=manifest_file)

    if options.classes_dir:
      # Delete the old classes directory. This ensures that all .class files in
      # the output are actually from the input .java files. For example, if a
      # .java file is deleted or an inner class is removed, the classes
      # directory should not contain the corresponding old .class file after
      # running this action.
      build_utils.DeleteDirectory(options.classes_dir)
      shutil.copytree(classes_dir, options.classes_dir)

  if options.depfile:
    build_utils.WriteDepfile(
        options.depfile,
        input_files + build_utils.GetPythonDependencies())

  if options.stamp:
    build_utils.Touch(options.stamp)


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))


