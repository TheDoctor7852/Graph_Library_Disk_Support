CXX_FLAGS := -Wno-unused-local-typedefs -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -Wno-unused -pthread  -Wall -Wno-deprecated -O3 -Wsign-compare   -std=c++17
#CXX_FLAGS := -pthread  -Wall -O3 -std=c++17
CXX_DEFINES := -DBOOST_ALL_NO_LIB -DBOOST_DATE_TIME_DYN_LINK -DHAS_BOOST -DUSE_LOGGING -DUSE_PMDK -DUSE_TX
HEADER_DIR := ../header
OBJ_DIR := ../obj
SRC_DIR := ../src
PATH_TO_POSEIDON_CORE := /home/jan/Dokumente/poseidon_core#hier der totale oder abstratkte Pfad zu Poseidon
POS_COR_SRC := $(PATH_TO_POSEIDON_CORE)/src#falls das src-Verzischniss umbenannt wurde auch hier umbenennen
POS_COR_BUILD := $(PATH_TO_POSEIDON_CORE)/build#falls das build-Verzischniss umbenannt(bzw. anders benannt) wurde auch hier umbenennen
INC := -I$(HEADER_DIR) -I$(POS_COR_SRC) -I$(POS_COR_SRC)/btree -I$(POS_COR_SRC)/dict -I$(POS_COR_SRC)/ldbc -I$(POS_COR_SRC)/log -I$(POS_COR_SRC)/pool -I$(POS_COR_SRC)/py -I$(POS_COR_SRC)/qop -I$(POS_COR_SRC)/storage -I$(POS_COR_SRC)/tx -I$(POS_COR_SRC)/graph_analytics/analytics -I$(POS_COR_SRC)/graph_analytics/utils -I$(POS_COR_BUILD)/_deps/spdlog-src/include -I$(POS_COR_BUILD)/_deps/csv-src -I$(POS_COR_BUILD)/_deps/catch-src/single_include/catch2 -I$(POS_COR_BUILD)/generated
LINKER_COMMANDS := -Wl,-rpath,$(POS_COR_BUILD) -L$(POS_COR_BUILD) -lposeidon_core -lpmemobj /usr/lib/x86_64-linux-gnu/libboost_date_time.so.1.71.0 -lpthread
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

all: $(OBJECTS)
	g++ $(CXX_FLAGS) $(CXX_DEFINES) $(INC) -shared -o libgraph_components.so $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	g++ $(CXX_FLAGS) $(CXX_DEFINES) $(INC) -c -fPIC -o $@ $<