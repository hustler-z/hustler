+-----------------------------------------------------------------------------+
| QUICK CHEATSHEET                                                            |
+-----------------------------------------------------------------------------+

$ grep [target] [path] -rn --exclude-dir [directory excluded] --exclude=[pattern]

-------------------------------------------------------------------------------
- PATCH -

(1) create a patch:

$ diff -ruN [previous source] [current source] > [patch name].patch

(2) apply a patch:

$ patch -p1 < path/to/[patch name].patch

(3) revert a patch:

$ patch -R -p1 < path/to/[patch name].patch

-------------------------------------------------------------------------------
- GIT USAGE -

(1) acquire all remote branches:

$ git remote update origin --prune
$ git fetch origin

(2) make a copy of the remote branch:

$ git checkout -b [branch name] origin/[branch name]

(3) use git add to add multiple modified files:

$ git add [file 0]
$ git add [file 1]
...

(4) use git push to submit changes to remote branch:

$ git push origin [branch name]

-------------------------------------------------------------------------------

                                fdisk => manipulate disk partition table
+----------------+               |
| storage device | -> parted -> mkfs -> mount
+----------------+               |
                                fsck => check and repair a Linux filesystem

-------------------------------------------------------------------------------

$ cat /proc/sys/kernel/printk => log level

4         4         1         7
current | default | minimum | boot-time-default

-------------------------------------------------------------------------------
