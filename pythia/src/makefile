SOFTWARE_PATH=../../software/

generate_root: generate_root.cc
	@g++ generate_root.cc -o run_root -w  -I $(SOFTWARE_PATH)/Pythia-8.312/include -O2 -std=c++11 -pedantic -W -Wall -Wshadow -fPIC -pthread  -L $(SOFTWARE_PATH)/Pythia-8.312/lib -Wl,-rpath,$(SOFTWARE_PATH)/Pythia-8.312/lib -lpythia8 -ldl -I$(SOFTWARE_PATH)/FastJet-3.4.2/fastjet-install/include -L$(SOFTWARE_PATH)/FastJet-3.4.2/fastjet-install/lib -Wl,-rpath,$(SOFTWARE_PATH)/FastJet-3.4.2/fastjet-install/lib -lfastjet -I$(SOFTWARE_PATH)/HepMC-2.06.11/hepmc-install/include -L$(SOFTWARE_PATH)/HepMC-2.06.11/hepmc-install/lib -Wl,-rpath,$(SOFTWARE_PATH)/HepMC-2.06.11/hepmc-install/lib -lHepMC -DHEPMC2 $(shell root-config --libs --cflags)

generate_hepmc: generate_hepmc.cc
	@g++ generate_hepmc.cc -o run_hepmc -w  -I $(SOFTWARE_PATH)/Pythia-8.312/include -O2 -std=c++11 -pedantic -W -Wall -Wshadow -fPIC -pthread  -L $(SOFTWARE_PATH)/Pythia-8.312/lib -Wl,-rpath,$(SOFTWARE_PATH)/Pythia-8.312/lib -lpythia8 -ldl -I$(SOFTWARE_PATH)/HepMC-2.06.11/hepmc-install/include -L$(SOFTWARE_PATH)/HepMC-2.06.11/hepmc-install/lib -Wl,-rpath,$(SOFTWARE_PATH)/HepMC-2.06.11/hepmc-install/lib -lHepMC -DHEPMC2

clean:
	@rm -f run_root
	@rm -f run_hepmc
