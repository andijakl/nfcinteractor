@ECHO OFF
SET _prjname=nfcinteractor

del nfcinteractorunlimited

del .make.cache
del bld.inf
del build-stamp
del configure-stamp
del Makefile
del %_prjname%
del %_prjname%.loc
del %_prjname%.rss
del %_prjname%_exe.mmp
del %_prjname%_installer.pkg
del %_prjname%_reg.rss
del %_prjname%_stub.pkg
del %_prjname%_template.pkg

rmdir /S /Q obj
rmdir /S /Q moc
rmdir /S /Q debian
