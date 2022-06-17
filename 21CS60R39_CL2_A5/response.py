
import ply.yacc as yacc
import ply.lex as lex
from datetime import datetime

class Response:
    def __init__(self):
        self.dict1 = {}
        self.tokens = [
            'NAME',
            'END',
            'LEFT',
            'RIGHT',
            'A',
            'C',
        ]
        self.year=None
        self.month=None

  
    def t_END(self, t):
        r'<span.class="mw-headline".id="Summary">'
        return t

    def t_A(self, t):
        r'<p>'
        return t

    def t_C(self, t):
        r'<li>'
        return t

    def t_LEFT(self, t):
        r'<span.class="mw\-headline".id="[0-9][a-zA-Z0-9\-_]*">'
        return t

    def t_RIGHT(self, t):
        r"</h3>"
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

    def t_NAME(self, t):
        r"[A-Za-ze̝0-9+':,.\/\[\]\-(_)#=()]+"
        return t

    def t_ANY(self, t):
        r'<[A-Za-ze̝0-9\/:\+\-\(\)\'×\[\]–=’"_!\t\n\{\},\.;%&#? ]+>'
        pass

    t_ignore = " \t"

    def t_error(self, t):
        t.lexer.skip(1)

    def p_start_mulit(self, t):
        '''start : start1 LEFT pname RIGHT data END
            '''
        # print('k ', t[3])
        date=t[3].strip().split(" ")[0]
        if date.isdigit():
            date=date+" "+self.month+" "+self.year
            d=datetime.strptime(date, '%d %B %Y').date() 
            if d not in self.dict1:
                self.dict1[d]=''
            self.dict1[d]+=t[5]
        
    def p_start(self, t):
        '''start : LEFT pname RIGHT data END
            '''
        # print('k ', t[2])
        date=t[2].strip().split(" ")[0]
        if date.isdigit():
            date=date+" "+self.month+" "+self.year
            d=datetime.strptime(date, '%d %B %Y').date() 
            if d not in self.dict1:
                self.dict1[d]=''
            self.dict1[d]+=t[4]
        
    def p_start1(self, t):
        '''start1 : start1 LEFT pname RIGHT data
            '''
        date=t[3].strip().split(" ")[0]
        if date.isdigit():
            date=date+" "+self.month+" "+self.year
            d=datetime.strptime(date, '%d %B %Y').date() 
            if d not in self.dict1:
                self.dict1[d]=''
            self.dict1[d]+=t[5]        
        
        # print('b ', t[3])

    def p_start2(self, t):
        '''start1 :  LEFT pname RIGHT data'''
        t[0] = t[4]
        date=t[2].strip().split(" ")[0]
        if date.isdigit():
            date=date+" "+self.month+" "+self.year
            d=datetime.strptime(date, '%d %B %Y').date() 
            if d not in self.dict1:
                self.dict1[d]=''
            self.dict1[d]+=t[4]
        # print('a', t[2])

    def p_data1(self, t):
        '''data : data temp A pname
                | data temp C pname'''
        t[0] = t[1]+' '+t[4]

    def p_data2(self, t):
        '''data : temp A pname
                | temp C pname'''
        t[0] = t[3]

    def p_temp(self, t):
        '''temp : pname
                | RIGHT
                |'''

    def p_pname_multi(self, t):
        'pname : pname NAME'
        t[0] = t[1]+' '+t[2]

    def p_pname(self, t):
        'pname : NAME'
        t[0] = t[1]

    def p_error(self, t):
        pass

    def crawl(self, file_name,year,month, d):
        self.year,self.month=year,month
        self.dict1 = d
        text = open(file_name, 'r', errors='ignore').read()
        self.lexer = lex.lex(object=self)
        self.lexer.input(str(text))
        # for tok in self.lexer:
        #   print(tok.type, tok.value)
        self.parser = yacc.yacc(module=self)
        self.parser.parse(text)
        return self.dict1

# dicti = {}
# covid = Response()
# covid.crawl('2020_January_response.html','2020','January', dicti)
# # print(covid.dict1)
# print(dicti)
# for k, x in covid.dict1.items():
#     print(k, len(x))


# x=datetime.strptime('3 January 2020', '%d %B %Y').date()
# print(covid.dict1[x])
