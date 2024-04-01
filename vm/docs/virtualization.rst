+---------------------------------------------------------+
| QUICK INTROS                                            |
+---------------------------------------------------------+

-----------------------------------------------------------
- INTRO -






-----------------------------------------------------------
- USAGE -

(1) qemu compilation

$ mkdir build && cd build
$ ../configure --enable-kvm --enable-debug --target-list=*
check help for available targets:
$ ../configure --help
$ make -j$(nproc)

(2) kvmtool compilation

$ make -j$(nproc)


-----------------------------------------------------------
