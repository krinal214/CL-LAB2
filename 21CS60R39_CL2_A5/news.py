
import ply.yacc as yacc
import ply.lex as lex
from datetime import datetime
import re


class News:
    def __init__(self):
        self.dict1 = {}
        self.tokens = [
            'NAME',
            'MON1',
            'MON2',
            'MON3',
            'MON4',
            'X',
            'END'
        ]
        self.year = None
        self.months = {'January': 1, 'February': 2, 'March': 3, 'April': 4, 'May': 5, 'June': 6,
                        'July': 7, 'August': 8, 'September': 9, 'October': 10, 'November': 11, 'December': 12}
        self.lastmonth = None
        self.mindate = None
        self.maxdate = None

    def t_X(self, t):
        r'<span.class="mw-headline".id="(January_1–20,_2020|January|February|March|April|May|June|July|August|September|October|November|December)(2019|2020|2021|2022|_|-|–|January|February|March|April|May|June|July|August|September|October|November|December|present)*">'
        return t

    def t_MON1(self, t):
        r"(On|Also\son|By|From|As\sof)\s[0-9]{1,2}.(January|February|March|April|May|June|July|August|September|October|November|December)\b"
        return t

    def t_MON3(self, t):
        r"(On|Also\son|By|From|As\sof)\s(January|February|March|April|May|June|July|August|September|October|November|December)\s[0-9]{1,2}\b"
        return t

    def t_MON2(self, t):
        r'<[A-Za-z0-9–\+\/:\-()="_!\t\n,\.;%&#? ]+>[0-9]{1,2}.(January|February|March|April|May|June|July|August|September|October|November|December)\b'
        return t

    def t_MON4(self, t):
        r'<[A-Za-z0-9–\+\/:\-()="_!\t\n,\.;%&#? ]+>(January|February|March|April|May|June|July|August|September|October|November|December)\s[0-9]{1,2}\b'
        return t

    def t_NAME(self, t):
        r"[A-Za-ze̝0-9–\+'\":,.\/\[\]\-(_)#=()]+"
        return t

    def t_END(self, t):
        r'<span.class="mw-headline".id="(See_also|References|Notes|Alert_levels_timeline|Traffic_lights_timeline|Timeline_of_new_cases_in_Russia_by_federal_subjects)">'
        return t

    def t_ANY1(self, t):
        r'&\#91;[0-9]*&\#93;'
        pass

    def t_ANY2(self, t):
        r'&\#91;'
        pass

    def t_ANY3(self, t):
        r'&\#93;'
        pass

    def t_ANY(self, t):
        r'<[A-Za-ze̝0-9\/:\+\-\(\)\'×\[\]–=’"_!\t\n\{\},\.;%&#? ]+>'
        pass
    t_ignore = " \t"

    def t_error(self, t):
        t.lexer.skip(1)

    def p_start(self, t):
        '''start1 : start END'''

    def p_start1(self, t):
        '''start : start X pname data
            '''
        # print('k', t[2])
        y = re.findall(r'2019|2020|2021|2022', t[2])
        # print("kn", len(y))
        if len(y) != 0:
            self.year = y[0]
        elif self.provided_year is None:
            self.update_year(t[2])
        for key, value in t[4].items():
            # print(key)
            date = datetime.strptime(key+' '+self.year, '%d %B %Y').date()
            if date in self.dict1:
                self.dict1[date] += value
            else:
                self.dict1[date] = value
        self.update_date_range(t[2])

    def p_start2(self, t):
        '''start : start X pname'''
        # print('k', t[2])
        y = re.findall(r'2019|2020|2021|2022', t[2])
        if len(y) != 0:
            self.year = y[0]
        elif self.provided_year is None:
            self.update_year(t[2])
        self.update_date_range(t[2])

    def p_start3(self, t):
        '''start : X pname data'''
        y = re.findall(r'2019|2020|2021|2022', t[1])
        # print("kn", len(y))
        if len(y) != 0:
            self.year = y[0]
        elif self.provided_year is None:
            self.update_year(t[1])
        for key, value in t[3].items():
            date = datetime.strptime(key+' '+self.year, '%d %B %Y').date()
            if date in self.dict1:
                self.dict1[date] += value
            else:
                self.dict1[date] = value
        self.update_date_range(t[1])
        # print('k', t[1])

    def p_start4(self, t):
        '''start : X pname'''
        # print('k', t[1])
        y = re.findall(r'2019|2020|2021|2022', t[1])
        # print("kn", len(y))
        if len(y) != 0:
            self.year = y[0]
        elif self.provided_year is None:
            self.update_year(t[1])
        self.update_date_range(t[1])

    def p_data1(self, t):
        '''data : data MON2 pname
            | data MON1 pname
            | data MON3 pname
            | data MON4 pname
        '''
        date = re.findall(
            r'[0-9]+\sJanuary|[0-9]+\sFebruary|[0-9]+\sMarch|[0-9]+\sApril|[0-9]+\sMay|[0-9]+\sJune|[0-9]+\sJuly|[0-9]+\sAugust|[0-9]+\sSeptember|[0-9]+\sOctober|[0-9]+\sNovember|[0-9]+\sDecember', t[2])
        flag = False
        if len(date) == 0:
            date = re.findall(
                r'January\s[0-9]+|February\s[0-9]+|March\s[0-9]+|April\s[0-9]+|May\s[0-9]+|June\s[0-9]+|July\s[0-9]+|August\s[0-9]+|September\s[0-9]+|October\s[0-9]+|November\s[0-9]+|December\s[0-9]+', t[2])
            flag = True
        d1 = date[0].split(' ')
        t[2] = t[2].strip()
        key = t[2]
        if t[2][0] == '<':
            key = t[2].split('>')[1]
        if flag:
            d1.reverse()
        date = d1[0]+' '+d1[1]
        t[0] = t[1]
        if date in t[0]:
            t[0][date] += key+' '+t[3]
        else:
            t[0][date] = key+' '+t[3]
        # print(t[2], ' ', d1)

    def p_data2(self, t):
        '''data : data MON2
            | data MON1
            | data MON3
            | data MON4
        '''
        date = re.findall(
            r'[0-9]+\sJanuary|[0-9]+\sFebruary|[0-9]+\sMarch|[0-9]+\sApril|[0-9]+\sMay|[0-9]+\sJune|[0-9]+\sJuly|[0-9]+\sAugust|[0-9]+\sSeptember|[0-9]+\sOctober|[0-9]+\sNovember|[0-9]+\sDecember', t[2])
        flag = False
        if len(date) == 0:
            date = re.findall(
                r'January\s[0-9]+|February\s[0-9]+|March\s[0-9]+|April\s[0-9]+|May\s[0-9]+|June\s[0-9]+|July\s[0-9]+|August\s[0-9]+|September\s[0-9]+|October\s[0-9]+|November\s[0-9]+|December\s[0-9]+', t[2])
            flag = True
        d1 = date[0].split(' ')
        if flag:
            d1.reverse()
        date = d1[0]+' '+d1[1]
        t[0] = t[1]
        # print(t[2], ' ', d1)

    def p_data3(self, t):
        '''data : MON2 pname
                | MON1 pname
                | MON3 pname
                | MON4 pname
                '''
        t[1] = t[1].strip()
        key = t[1]
        if t[1][0] == '<':
            key = t[1].split('>')[1]
        date = re.findall(
            r'[0-9]+\sJanuary|[0-9]+\sFebruary|[0-9]+\sMarch|[0-9]+\sApril|[0-9]+\sMay|[0-9]+\sJune|[0-9]+\sJuly|[0-9]+\sAugust|[0-9]+\sSeptember|[0-9]+\sOctober|[0-9]+\sNovember|[0-9]+\sDecember', t[1])
        flag = False
        if len(date) == 0:
            date = re.findall(
                r'January\s[0-9]+|February\s[0-9]+|March\s[0-9]+|April\s[0-9]+|May\s[0-9]+|June\s[0-9]+|July\s[0-9]+|August\s[0-9]+|September\s[0-9]+|October\s[0-9]+|November\s[0-9]+|December\s[0-9]+', t[1])
            flag = True
        d1 = date[0].split(' ')
        if flag:
            d1.reverse()
        date = d1[0]+' '+d1[1]
        t[0] = {}
        t[0][date] = key+' '+t[2]
        # print(t[1], ' ', d1)

    def p_data4(self, t):
        '''data : MON2
                | MON1
                | MON3
                | MON4
                '''
        date = re.findall(
            r'[0-9]+\sJanuary|[0-9]+\sFebruary|[0-9]+\sMarch|[0-9]+\sApril|[0-9]+\sMay|[0-9]+\sJune|[0-9]+\sJuly|[0-9]+\sAugust|[0-9]+\sSeptember|[0-9]+\sOctober|[0-9]+\sNovember|[0-9]+\sDecember', t[1])
        flag = False
        if len(date) == 0:
            date = re.findall(
                r'January\s[0-9]+|February\s[0-9]+|March\s[0-9]+|April\s[0-9]+|May\s[0-9]+|June\s[0-9]+|July\s[0-9]+|August\s[0-9]+|September\s[0-9]+|October\s[0-9]+|November\s[0-9]+|December\s[0-9]+', t[1])
            flag = True
        d1 = date[0].split(' ')
        if flag:
            d1.reverse()
        date = d1[0]+' '+d1[1]
        t[0] = {}
        # print(t[1], ' ', d1)

    def p_pname_multi(self, t):
        'pname : NAME pname'
        t[0] = t[1]+' '+t[2]

    def p_pname(self, t):
        'pname : NAME'
        t[0] = t[1]

    def p_error(self, t):
        pass

    def update_year(self, msg):
        for i in self.months.keys():
            if re.search(i, msg):
                if self.lastmonth is None:
                    self.lastmonth = i
                elif self.months[self.lastmonth] >= self.months[i]:
                    self.year = str(int(self.year)+1)
                    self.lastmonth = i
                else:
                    self.lastmonth = i
                break

    def update_date_range(self, msg):
        for key in self.months.keys():
            if re.search(key, msg):
                d = datetime.strptime(key+' '+self.year, '%B %Y').date()
                if self.mindate is None:
                    self.mindate = d
                elif self.mindate > d:
                    self.mindate = d
                if self.maxdate is None:
                    self.maxdate = d
                elif self.maxdate < d:
                    self.maxdate = d

    def crawl(self, file_name, year, provided_year,d1,d2, d):
        self.year = year
        self.dict1 = d
        self.mindate=d1
        self.maxdate=d2
        self.provided_year = provided_year
        text = open(file_name, 'r', errors='ignore').read()
        self.lexer = lex.lex(object=self)
        self.lexer.input(str(text))
        # for tok in self.lexer:
        #   print(tok.type, tok.value)
        self.parser = yacc.yacc(module=self)
        self.parser.parse(text)


'''
covid = News()
d = {}
d['temp'] = {}
d1,d2=None,None
covid.crawl('temp.html', '2020', None,d1,d2, d['temp'])
# print(covid.dict1)
for k, x in d['temp'].items():
    print(k, len(x))

print(covid.mindate,' ',covid.maxdate)
x = datetime.strptime('7 January 2021', '%d %B %Y').date()
print(d['temp'][x])
'''
