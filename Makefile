CC=gcc
CFLAGS=-c
LDFLAGS=

#Directories
OBJ_DIR=obj/
SRC_DIR=src/
INC_DIR=inc/
BIN_DIR=bin/

MTCA_INC=./MTCALib/inc/
MTCA_LIB=./MTCALib/lib/

#Files ------------------------------------------------------------------------

#MTCALib
MTCALIB_LIB = ./MTCALib/lib/libmtca.a

#HPMDownloder
HPMDOWNLOADER_SRC= $(wildcard $(SRC_DIR)*.c)
HPMDOWNLOADER_OBJ=$(HPMDOWNLOADER_SRC:$(SRC_DIR)%.c=$(OBJ_DIR)%.o)

#Libraries
HPMDOWNLOADER_BIN= hpmdownloader.exe

#Rules ------------------------------------------------------------------------
all: $(MTCALIB_LIB) $(HPMDOWNLOADER_OBJ) $(HPMDOWNLOADER_BIN)

$(MTCALIB_LIB):
	@echo "Construction of MTCALib "
	@make -s -C ./MTCALib

$(HPMDOWNLOADER_OBJ): $(OBJ_DIR)%.o : $(SRC_DIR)%.c
	@echo "Construction of $@ from $<"
	$(CC) $(CFLAGS) -I $(INC_DIR) -I $(MTCA_INC) $< -o $@
	@echo ""

$(HPMDOWNLOADER_BIN) : $(HPMDOWNLOADER_OBJ)
	@echo "Construction of the HPMDownloader executable"
	gcc -L $(MTCA_LIB) -o $(BIN_DIR)$(HPMDOWNLOADER_BIN) $(HPMDOWNLOADER_OBJ) -lmtca -lcrypto -lssl
	@echo ""

clean: mrproper
	@echo "removing objects"
	-rm $(OBJ_DIR)*.o
	@echo ""

	@echo "removing exec"
	-rm $(BIN_DIR)$(HPMDOWNLOADER_BIN)
	@echo ""

mrproper:
	@echo "Removing all *~ files"
	-find . -name "*~" -exec rm {} \;
	@echo ""
