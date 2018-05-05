File Structure and what not
  -build folder is where all the build files go
  -mmo folder has the shared mmo library files
  -doxygen folder is where the doxygen output goes, doxygen currently only runs on the mmo directory
  -every other folder ie load_balancer, zone, database_app, ... have files specific to a certain
   executable. for example any file with a main function will go in its own folder.
  -The only difference between all the make files is the executable name, so when making a new
   executable make a new folder add the code files and copy paste a make file and change the
   EXE variable
   
