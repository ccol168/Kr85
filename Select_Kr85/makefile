
LIBS:=`root-config --libs`
INCS:=`root-config --cflags`

do:	
	make JVertex_Kr85_reader

JVertex_Kr85_reader: JVertex_Kr85_reader.o
	g++ JVertex_Kr85_reader.o -o JVertex_Kr85_reader.x -lSpectrum ${INCS} ${LIBS}

JVertex_Kr85_reader.o: JVertex_Kr85_reader.cxx
	g++ -c JVertex_Kr85_reader.cxx -o JVertex_Kr85_reader.o -lSpectrum ${INCS} ${LIBS}
