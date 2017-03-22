#!/usr/bin/env python
# -*- coding: windows-1251 -*-

#  Copyright (C) 2012 Mohamed M. El-Kalioby
#  All rights reserved.
# 
# 
#  THIS SOFTWARE IS PROVIDED BY Mohamed El-Kalioby ``AS IS'' AND ANY
#  EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL Roman V. Kiseliov OR
#  ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
#  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
#  OF THE POSSIBILITY OF SUCH DAMAGE.

__rev_id__ = """$Id: setup.py,v 1.10 2005/10/26 07:44:23 rvk Exp $"""

import sys
from distutils.core import setup

DESCRIPTION = \
'Create and communicate with a cluster on Amazon Web Service (AWS)'

LONG_DESCRIPTION = \
''''''

CLASSIFIERS = \
[
 'Operating System :: OS Independent',
 'Programming Language :: Python',
 'License :: GNU v.3',
 'Development Status :: 5 - Productions',
 'Intended Audience :: Developers',
 'Topic :: Software Development :: Libraries :: Python Modules',
 'Topic :: AWS :: Cloud',
 ]

KEYWORDS = \
'AWS Cluster Bioinf'

setup(name = 'HPC-cloud-client',
      version = '0.75',
      author = 'Mohamed El-Kalioby',
      author_email = 'mkalioby@mkalioby.com	',
      url = 'http://www.hpc-cloud.org',
      download_url='',    
      description = DESCRIPTION,
      long_description = LONG_DESCRIPTION,
      license = 'GNU',
      platforms = 'Platform Independent',
      packages = ['HPC-cloud-client'],
      keywords = KEYWORDS,
      classifiers = CLASSIFIERS
      )
