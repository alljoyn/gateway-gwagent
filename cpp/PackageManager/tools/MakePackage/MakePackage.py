#!/usr/bin/env python

#given a directory containing an application and a signing certificate private key file, create an Alljoyn Package

# An Alljoyn (AJ) package consists of a directory with the same name as the package.  
# This directory will be directly copied into the target Alljoyn device.  The Package Manager
# will not execute any code contained in the package directory.
#
# The top level of the package directory contains the application's Manifest.xml.
# Inside the package directory should be subdirectories containing the executable, libraries,
# and any other resources required by the application.  Executable code should be in a /bin
# subdirectory, required libraries should be in a /lib subdirectory.  Any required environmental variables
# are described  by the manifest file and implemented by the Gateway Application Manager. 
#
# The AJ package is created as follows.
# 1) Create a compressed tarball containing the package directory ( packagename.tar.gz ).  This is the 'inner tarball'.
# 2) Created a signed hash of this 'inner tarball' ( packagename.tar.gz.sha1 ) using openssl, encrypted with the private key of the
#    signing certificate in .pem format
#    Note that this same signing certificate must be installed in the certificate store of the target device.  The private key pass phrase 
#    should be stripped from the device copy of the certificate.
# 3) Create a tarball containing these to files into the 'outer tarball' ( packagename.tar ).  
# 4) The outer tarball is made available on an https server for download by the Package Manager.

import os

def tar_dir(output_filename, source_dir):
	cmd = "tar cvzf " + output_filename + " " + source_dir
	print cmd
	os.system(cmd)
		
def verify_file_exists(path) :
	if not os.path.isfile(path) :
		print 'unable to find ' + path
		os._exit(1)

# get top directory of package
packageDir =  raw_input('Enter path to package directory: ')
packageDir.rstrip()
if packageDir.endswith('/') :
	packageDir = packageDir[:len(packageDir)-1]
packageName = os.path.split(packageDir)[1]
print 'Preparing AJ package:' + packageName

#tar and compress the package directory
print 'Compressing ' + packageDir
packageTar = packageName + '.tar.gz'
tar_dir(packageTar, packageDir) 
if not os.path.isfile(packageTar) :
	print 'unable to create ' + packageTar
	os._exit(1)

#compute the signed sha1 hash of the package directory, save it as packageName.tar.gz.sha1
privateKey = raw_input('enter path to private key file of the signing certificate (pem format):')

verify_file_exists(privateKey)

sslcmd =  'openssl dgst -sha1 -sign ' + privateKey + ' -out ' + packageTar + '.sha1 ' + packageTar
print 'ssl command:' + sslcmd
os.system(sslcmd) 

verify_file_exists(packageTar + '.sha1')

#create the 'outer tar', i.e. the AJ format package suitable for downloading by the AJ package manager
cmd = 'tar cf ' + packageName + '.tar ' + packageTar + ' ' + packageTar + '.sha1'
print cmd;
os.system(cmd)

verify_file_exists(packageName + '.tar')

print 'Created AJ package: ' + 	packageName + '.tar'


