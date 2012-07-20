# ================================================================
# John Kerl
# kerl.john.r@gmail.com
# 2010-02-02
# ================================================================

opt:
	export OPTCFLAGS="-O3" OPTLFLAGS=""; make -ef count_walks.mk

profile:
	export OPTCFLAGS="-g -O3 -pg" OPTLFLAGS="-g -pg"; make -ef count_walks.mk

gcov:
	export OPTCFLAGS="-g -fprofile-arcs -ftest-coverage" OPTLFLAGS="-g -fprofile-arcs -ftest-coverage"; make -ef count_walks.mk

debug:
	export OPTCFLAGS="-g" OPTLFLAGS="-g"; make -ef count_walks.mk

build:
	make -f count_walks.mk

mk:
	yamm count_walks.mki

install:
	make -f count_walks.mk  install

clean:
	make -f count_walks.mk  clean

dataclean:
	@rm -f w_theta_with_N_series.txt  w_ratio_theta_with_N_series.txt w_ratio_N_with_theta_series.txt
	@rm -f w_N_with_theta_series.txt  bw_theta_with_N_series.txt      bw_N_with_theta_series.txt
	@rm -f bc_theta_with_N_series.txt bc_N_with_theta_series.txt      aw_theta_with_N_series.txt
	@rm -f aw_N_with_theta_series.txt ac_theta_with_N_series.txt      ac_N_with_theta_series.txt

tags: .PHONY
	ctags *.[ch]

.PHONY:

over: clean mk build
