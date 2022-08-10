#!/bin/bash
##https://softwareengineering.stackexchange.com/questions/165725/git-branching-and-tagging-best-practices
## this script has been tested on gnu-bash on Linux and git-bash for windows
if (echo a version 2.8.0; git --version) | sort -Vk3 | tail -1 | grep -q git
then
    echo "Using: `git --version`"
else
    echo "Not good: min version of git required is 2.8.0"
	exit 1;
fi
## Use this is you run the script from a directory other than the source code dir
# if [[ "$OSTYPE" == "linux-gnu"* ]]; then
	# srcdir=./;
# elif [[ "$OSTYPE" == "cygwin" ]]; then
	# srcdir=/c/Users/Arijit.Khan/Source/Repos/Basin-Modelling
# elif [[ "$OSTYPE" == "msys" ]]; then
	# srcdir=/c/Users/Arijit.Khan/Source/Repos/Basin-Modelling
# else
    # srcdir=./;    # Unknown.
# fi

srcdir=./;
## the following is not yet used
defFormat='%s%n%n%b%n';

echo "Using the source directory location as "$srcdir
#############################################
# https://pypi.org/project/gitchangelog/
# git commands may fail to accss remote branches if you are using a Shell proxy and not connected to Shell network
#############################################
# update-ChangeLog:
    if test -d $srcdir/.git; then                         						
		git -C $srcdir status	
		#git rev-parse HEAD
		#git ls-remote <url> <refs>		
		sha1=`cat last-cl-gen`
		sha1_latest=`git -C $srcdir rev-list -n 1 HEAD`;
		# avoid mutiple updates, Comparison
		if [ "$sha1" != "$sha1_latest" ]; then
			./perlSrForChangelog            														\
				--srcdir=$srcdir																	\
				--format="%s%n%bCommit-ID:<%H>%x09 %d" --no-cluster             					\
				--ignore-matching=Revert															\
				--strip-tab --strip-cherry-pick  --append-dot 										\
				-- $sha1...HEAD --no-merges --grep=new: --grep=chg: --grep=fix: --decorate=full 	\
				--simplify-by-decoration --decorate-refs=refs/tags	--decorate-refs=refs/heads		\
				>Change.log.tmp;	
			##_file="$Change.log.tmp"
			###[ $# -eq 0 ] && { echo "Usage: $0 filename"; exit 1; }
			###[ ! -f "$_file" ] && { echo "Error: $0 file not found."; exit 2; }
 
			if [ -s ./Change.log.tmp ]; then
				# do something as file has data
				echo -e "\n++" >> Change.log.tmp;
				echo "Updating Change.log"
				echo "$sha1_latest" > last-cl-gen.tmp 
				################################################################################################################################################
				git -C $srcdir fetch --all
				## to know all about git tags: https://stackoverflow.com/questions/35979642/what-is-git-tag-how-to-create-tags-how-to-checkout-git-remote-tags
				## after you have created a tagged commit you will have to share it will all by: git push origin v2.0.1 to sare the tad named v2.0.1 only or 
				## git push origin --tags, to share all tags created by you
				#################################################################################################################################################
				## the following will check for global tags across all branches that was shared and that will be our version
				#vers=`git -C $srcdir rev-list --tags --timestamp --no-walk | sort -nr | head -n1 | cut -f 2 -d ' ' | xargs git -C $srcdir describe --contains`
				#vers=`git -C $srcdir describe --tags $(git -C $srcdir rev-list --tags --max-count=1)`
				TAGS=`git -C $srcdir tag`
				if [ ${#TAGS[@]} -ge 0 ]; then
				git -C $srcdir fetch --tags
				vers=`git -C $srcdir describe --abbrev=0`
					if test -z "$vers"; then
						echo "no version tag in this branch!"
					else
						echo "This is the latest version you tagged-"$vers	
						date=`git -C $srcdir show -s --format=%ci $vers^{commit}`
						sed -i -e "1s/.*/Cauldron-${vers}	${date}/" VERSION
						sed -i "16s/.*/Cauldron-${vers}		${date}/" Change.log
					fi
				fi
				## Start appending to the top at line no. 38 
				line=38;	
				sed -i -e "${line}r Change.log.tmp" Change.log
				#cat Change.log           
				mv -f last-cl-gen.tmp last-cl-gen 
			else
				echo "No matching conditions for this commit message inorder to update change.log... check commit message standards; commit ID:" $sha1_latest	
				rm -f Change.log.tmp; exit 4 				
			fi			
			rm -f Change.log.tmp;  
		else
			echo "sha1 matches, same commit needs no updating!"
			rm -f Change.log.tmp; exit 3;
		fi
	fi

#EXTRA_DIST += last-cl-gen
#TIPs
#To delete remote tags (before deleting local tags) simply do:
#git tag -l | xargs -n 1 git push --delete origin
#and then delete the local copies:
#git tag | xargs git tag -d
