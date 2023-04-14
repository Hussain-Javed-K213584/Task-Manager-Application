Taskmgr: main.cpp
	g++ -o Taskmgr -Isqlite_modern_cpp/hdr bprinter/src/table_printer.cpp main.cpp sha256/sha256.cpp -lsqlite3