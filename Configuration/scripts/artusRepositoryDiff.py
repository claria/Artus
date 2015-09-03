#!/usr/bin/env python
# -*- coding: utf-8 -*-

import logging
import Artus.Utility.logger as logger
log = logging.getLogger(__name__)

import argparse
import os
import shlex
import subprocess

import Artus.Utility.jsonTools as jsonTools
import Artus.Utility.tools as tools


def main():
	parser = argparse.ArgumentParser(description="Compare different repository versions configured in Artus configs. The script has to be executed in the directory of the repository.", parents=[logger.loggingParser])
	parser.add_argument("files", help="Two configurations. The configs should be ordered by the expected repository version. Can be either Artus output root files or JSON text files", nargs=2)

	args = parser.parse_args()
	logger.initLogger(args)
	
	config1 = jsonTools.JsonDict(args.files[0])
	config2 = jsonTools.JsonDict(args.files[1])
	
	dirname = os.path.basename(os.getcwd())
	repo_key1 = sorted([key for key in config1.keys() if key.startswith("/") and key.endswith(dirname)], key=lambda item: len(item))[-1]
	repo_key2 = sorted([key for key in config2.keys() if key.startswith("/") and key.endswith(dirname)], key=lambda item: len(item))[-1]
	
	repo_version1 = config1[repo_key1]
	repo_version2 = config2[repo_key2]
	diff_string = "%s...%s" % (repo_version1, repo_version2)
	
	command = "git diff %s..%s" % (repo_version1, repo_version2)
	if log.isEnabledFor(logging.DEBUG):
		log.info("")
		logger.subprocessCall(shlex.split(command))
	log.info("\n"+command)
	
	popen_cout, popen_cerr = subprocess.Popen("git config remote.origin.url".split(), stdout=subprocess.PIPE).communicate()
	remote_url = popen_cout.replace("\n", "")
	github_link = os.path.join(remote_url, "compare", diff_string)
	log.info(github_link)
	

if __name__ == "__main__":
	main()

