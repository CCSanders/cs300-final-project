all: lib_cp 
	gcc -std=c99 -D_GNU_SOURCE -lpthread -lrt search_manager.c -o searchmanager

lib_cp: jni_header
	gcc -c -fPIC -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux system5_msg.c -o edu_cs300_MessageJNI.o
	gcc -shared -o libsystem5msg.so edu_cs300_MessageJNI.o -lc

jni_header: java_cp
	javac -h . edu/cs300/MessageJNI.java

java_cp:
	javac edu/cs300/*java
	javac CtCILibrary/*.java