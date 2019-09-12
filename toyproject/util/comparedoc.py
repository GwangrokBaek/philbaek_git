# The program to find the different part of the sentence between              #
# two files which are your targets. You should tell the program               #
# hint which is the first of the word of the sentence.                        #
#                                                                             #
# You can edit and use the code freely only if you state the original author. #
# Author : phil (100@cpguard.kr)                                              #

import re

# @brief Temporary space to store the data.
#
class File():
    def __init__(self):
        self.loc1 = 0           #!< The path of the first file location
        self.loc2 = 0           #!< The path of second file location
        self.line1 = 0          #!< The place to store raw data of the first file
        self.line2 = 0          #!< The place to store raw data of second file
        self.diff = False       #!< The flag to check whether files are different
        print("{0:*^50s}".format('Compare documents'))
        print("{0}".format('You should type pull path of the file location!'))
        print("{0}".format('After that, Type the exact word which \
is included in the sentence that you want to compare.\n'))

# @brief Function for receiving the location of the file.
# NOTE: fileloc1, fileloc2 should be the full path of the file location.
#
    def inputname(self):
        fileloc1 = input("Type the location of file1 : ")
        fileloc2 = input("Type the location of file2 : ")
        self.loc1 = fileloc1
        self.loc2 = fileloc2

# @brief Function for opening the file.
# @return The file pointer.
#
    def openfile(self):
        f1 = open(self.loc1, 'r')
        f2 = open(self.loc2, 'r')
        return (f1, f2)

# @brief Function for comparing the diefferent part of the sentence.
# @param[in]    arr1   The list which contains the sentence that you want to compare
# @param[in]    arr2    same with arr1
#
    def ismatch(self, arr1, arr2):
        if arr1[-1] != arr2[-1]:
            print('{0} \n {1} : {2} \n {3} : {4}'.format('\nmismatch found!', \
                                                  self.loc1[-50:], \
                                                  arr1, \
                                                  self.loc2[-50:], \
                                                  arr2))
            self.diff = True

# @brief Function for reading whole lines of the file.
# @param[in]    f1  The file pointer.
# @param[in]    f2  Same with f1.
#
    def readfile(self, f1, f2):
        self.line1 = f1.read()
        self.line2 = f2.read()
        string = input('Type the first word of the sentence which you want \
to compare : ')
        string = string + '.*'
        temp1 = re.findall(string, self.line1)
        temp2 = re.findall(string, self.line2)
        for array1 in temp1:
            for array2 in temp2:
                if array1[:-2] == array2[:-2]:
                    File.ismatch(self, array1, array2)
        print('{0}'.format('\nComparing work is done!\n'))
        if self.diff is False:
            print('There is no difference\n')

 # @brief Function for comparing the each sentence of the file sequentially.
 # @param[in]   f1  The file pointer.
 # @param[in]   f2  Same with f1.
 #
    def compare(self, f1, f2):
        lineno = 1
        while True:
            self.line1 = f1.readline()
            self.line2 = f2.readline()
            if self.line1 != self.line2:
                print('{0} {1}\n {2} : {3} \n {4} : {5}'.format('mismatch \
                     found :', lineno, self.loc1[-30:], self.line1, self.loc2[-30:], \
                         self.line2))
            lineno += 1

if __name__ == '__main__':
    try:
        while True:
            files = File()
            files.inputname()
            file1, file2 =files.openfile()
            files.readfile(file1, file2)
            file1.close()
            file2.close()
        
    except FileNotFoundError as exc:
        print(exc)
