TARGET		= ../../lib/libkdbapi.so
OBJPATH 	= .
SOURCES		= $(wildcard *.cpp)
OBJS    	= $(patsubst %.cpp,%.o,$(SOURCES))
FULLOBJS 	= $(patsubst %.cpp,$(OBJPATH)/%.o,$(SOURCES))
INCPATH		= -I../../include -I../../include/sqlite -I../../include/mysql
LIBPATH		= -L../../lib -lmysqlclient -lsqlite3
LFLAGS      	= -w -shared
CFLAGS      	= -w -c 
DFLAGS		= -DHAS_MYSQL -DHAS_SQLITE
CXX     	= g++

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CXX) $(LFLAGS) $(FULLOBJS) -o $(TARGET) $(LIBPATH)

$(OBJS):$(SOURCES)
	$(CXX) $(CFLAGS) $(DFLAGS) $*.cpp -o $(OBJPATH)/$@ $(INCPATH)

clean:
	rm -f $(OBJPATH)/*.o
	rm -f $(TARGET)
