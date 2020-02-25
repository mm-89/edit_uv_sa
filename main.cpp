#include "mainwindow.h"
#include <QApplication>
#include "IOCSV.h"
#include <typeinfo>
#include <iostream>

#include "IOFile.h"

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();

/*
	IOFile myFile;
	MeshDocument *myMeshDoc;
	const char *myChar = "ciao";

	myMeshDoc = new MeshDocument;
	myChar = new char;
	
	MeshModel *a = IOFile::loadFromFile( myChar , myMeshDoc);	

	return 0;
*/
}
