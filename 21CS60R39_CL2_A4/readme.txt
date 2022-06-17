1) I have modified worldometers_countrylist.txt. Hence, Kindly use provided file to run the task1.
Modifications 
removed : at the end of continent
changed Ocenia to Australia/Oceania (Reason: in the webpage it is Australia/Ocenia hence become easy to map in dictionary)

2) Run task1.py before running task2.py (worldata.py and countrydata.py contains grammar rules only)

3) It will take a time during initialization of task2
Reason: It will crawl main.html(home page of wordometer website) and webpages of 55 countries during initialization.

4) You can go back to previous page as well as home page from any page
Restriction: You cannot go back to previous page from home page

5)for % change queries:
if given date is < minimum date available 
then it will be considered 0 cases

if given date is > maximum date
then it will be considered invalid
