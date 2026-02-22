#include "http/brook_http_request.h"


/* Tokens as defined by rfc 2616. Also lowercases them.
 *        token       = 1*<any CHAR except CTLs or separators>
 *     separators     = "(" | ")" | "<" | ">" | "@"
 *                    | "," | ";" | ":" | "\" | <">
 *                    | "/" | "[" | "]" | "?" | "="
 *                    | "{" | "}" | SP | HT
 */
static const char tokens[256] = {
/*   0 nul    1 soh    2 stx    3 etx    4 eot    5 enq    6 ack    7 bel  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*   8 bs     9 ht    10 nl    11 vt    12 np    13 cr    14 so    15 si   */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  16 dle   17 dc1   18 dc2   19 dc3   20 dc4   21 nak   22 syn   23 etb */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  24 can   25 em    26 sub   27 esc   28 fs    29 gs    30 rs    31 us  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  32 sp    33  !    34  "    35  #    36  $    37  %    38  &    39  '  */
        0,      '!',      0,      '#',     '$',     '%',     '&',    '\'',
/*  40  (    41  )    42  *    43  +    44  ,    45  -    46  .    47  /  */
        0,       0,      '*',     '+',      0,      '-',     '.',      0,
/*  48  0    49  1    50  2    51  3    52  4    53  5    54  6    55  7  */
      '0',     '1',     '2',     '3',     '4',     '5',     '6',     '7',
/*  56  8    57  9    58  :    59  ;    60  <    61  =    62  >    63  ?  */
      '8',     '9',      0,       0,       0,       0,       0,       0,
/*  64  @    65  A    66  B    67  C    68  D    69  E    70  F    71  G  */
        0,      'a',     'b',     'c',     'd',     'e',     'f',     'g',
/*  72  H    73  I    74  J    75  K    76  L    77  M    78  N    79  O  */
      'h',     'i',     'j',     'k',     'l',     'm',     'n',     'o',
/*  80  P    81  Q    82  R    83  S    84  T    85  U    86  V    87  W  */
      'p',     'q',     'r',     's',     't',     'u',     'v',     'w',
/*  88  X    89  Y    90  Z    91  [    92  \    93  ]    94  ^    95  _  */
      'x',     'y',     'z',      0,       0,       0,      '^',     '_',
/*  96  `    97  a    98  b    99  c   100  d   101  e   102  f   103  g  */
      '`',     'a',     'b',     'c',     'd',     'e',     'f',     'g',
/* 104  h   105  i   106  j   107  k   108  l   109  m   110  n   111  o  */
      'h',     'i',     'j',     'k',     'l',     'm',     'n',     'o',
/* 112  p   113  q   114  r   115  s   116  t   117  u   118  v   119  w  */
      'p',     'q',     'r',     's',     't',     'u',     'v',     'w',
/* 120  x   121  y   122  z   123  {   124  |   125  }   126  ~   127 del */
      'x',     'y',     'z',      0,      '|',      0,      '~',       0 };

static const uint8_t normal_url_char[32] = {
/*   0 nul    1 soh    2 stx    3 etx    4 eot    5 enq    6 ack    7 bel  */
        0    |   0    |   0    |   0    |   0    |   0    |   0    |   0,
/*   8 bs     9 ht    10 nl    11 vt    12 np    13 cr    14 so    15 si   */
        0    | T(2)   |   0    |   0    | T(16)  |   0    |   0    |   0,
/*  16 dle   17 dc1   18 dc2   19 dc3   20 dc4   21 nak   22 syn   23 etb */
        0    |   0    |   0    |   0    |   0    |   0    |   0    |   0,
/*  24 can   25 em    26 sub   27 esc   28 fs    29 gs    30 rs    31 us  */
        0    |   0    |   0    |   0    |   0    |   0    |   0    |   0,
/*  32 sp    33  !    34  "    35  #    36  $    37  %    38  &    39  '  */
        0    |   2    |   4    |   0    |   16   |   32   |   64   |  128,
/*  40  (    41  )    42  *    43  +    44  ,    45  -    46  .    47  /  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  48  0    49  1    50  2    51  3    52  4    53  5    54  6    55  7  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  56  8    57  9    58  :    59  ;    60  <    61  =    62  >    63  ?  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |   0,
/*  64  @    65  A    66  B    67  C    68  D    69  E    70  F    71  G  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  72  H    73  I    74  J    75  K    76  L    77  M    78  N    79  O  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  80  P    81  Q    82  R    83  S    84  T    85  U    86  V    87  W  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  88  X    89  Y    90  Z    91  [    92  \    93  ]    94  ^    95  _  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  96  `    97  a    98  b    99  c   100  d   101  e   102  f   103  g  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/* 104  h   105  i   106  j   107  k   108  l   109  m   110  n   111  o  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/* 112  p   113  q   114  r   115  s   116  t   117  u   118  v   119  w  */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/* 120  x   121  y   122  z   123  {   124  |   125  }   126  ~   127 del */
        1    |   2    |   4    |   8    |   16   |   32   |   64   |   0, };

/**
 * Function to parse http data from buffers
 */
int
brook_http_parse ( brook_http_parse_t* parser, unsigned char* data, size_t len ) {

  for ( int i = 0; i < len; i++ ) {

    unsigned char ch = data[i];

    switch (parser->state) {

      case s_req_start:
      {
        if ( ch == '\r' || ch == '\n' ) {
          return BROOK_ERROR;
        }
        parser->index = 1;
        parser->method = 0;
        switch (tokens[ch]) {
          case 'g': parser->method = GET; break;
          case 'p': parser->method = POST; /* in this time of validiti can be POST or PUT */ break;
          case 'd': parser->method = DELETE; break;
          default: return BROOK_ERROR;
        }
        parser->state = s_req_method;
        break;
      }

      case s_req_method:
      {
        char c = tokens[ch];

        if ( parser->method == GET ) {
          if (parser->index == 1 && c == 'e') {
          }
          else if (parser->index == 2 && c == 't') {
            parser->state = s_req_url;
          } else {
            return BROOK_ERROR;
          }
        } else if ( parser->method == POST ) {
          if (parser->index == 1 && c == 'u') {
            parser->method = PUT;
          } else if (parser->index == 1 && c == 'o') {
          } else if (parser->index == 2 && c == 's') {
          } else if (parser->index == 3 && c == 't') {
            parser->state = s_req_url;
          } else {
            return BROOK_ERROR;
          }
        } else if ( parser->method == PUT ) {
          if (parser->index == 2 && c == 't') {
            parser->state = s_req_url;
          } else {
            return BROOK_ERROR;
          }
        } else if ( parser->method == DELETE ) {
          if (parser->index == 1 && c == 'e') {
          } else if (parser->index == 2 && c == 'l') {
          } else if (parser->index == 3 && c == 'e') {
          } else if (parser->index == 4 && c == 't') {
          } else if (parser->index == 5 && c == 'e') {
            parser->state = s_req_url;
          } else {
            return BROOK_ERROR;
          }
        }

        ++parser->index;
        if ( parser->state != s_req_method ) {
          parser->index = 0;
        }
        break;
      }

      case s_req_url:
      {
        if ( ch == ' ' || ch == '\t' ) {
          if ( parser->index == 0 ) {
            parser->index = 1;
          } else {
            parser->state = s_req_minor;
          }
        } else {
          if ( parser->index == 1 ) {
            if ( ch == '/' ) {
              parser->index = 2;
            } else {
              printf("invalid url start '/'\n");
              return BROOK_ERROR;
            }
          } else if ( !IS_URL_CHAR(ch) ) {
            printf("invalid tokens url\n");
            return BROOK_ERROR;
          }
        }

        if ( parser->state != s_req_url ) {
          parser->index = 0;
        }
        break;
      }

      case s_req_minor:
      {
        if ( parser->index == 0 && ch == 'H' ) {
        } else if ( parser->index == 1 && ch == 'T' ) {
        } else if ( parser->index == 2 && ch == 'T' ) {
        } else if ( parser->index == 3 && ch == 'P' ) {
        } else if ( parser->index == 4 && ch == '/' ) {
        } else if ( parser->index == 5 && ch == '1' ) {
        } else if ( parser->index == 6 && ch == '.' ) {
        } else if ( parser->index == 7 && ch == '1' ) {
          parser->http_minor = 1;
        } else if ( parser->index == 8 && ch == '\r' ) {
        } else if ( parser->index == 9 && ch == '\n' ) {
          parser->state = s_req_header_field_start;
        } else {
          printf("invalid HTTP version\n");
          return BROOK_ERROR;
        }

        ++parser->index;
        if ( parser->state != s_req_minor ) {
          parser->index = 0;
        }
        break;
      }

      case s_req_header_field_start:
      {
        if (ch == ' ' || ch == '\t') {
          break;
        }

        if ( parser->index == 0 && ch == '\r' ) {
          parser->index = 1;
          break;
        } else if ( parser->index == 1 && ch == '\n' ) {
          parser->index = 0;
          parser->state = s_req_headers_done;
          printf("terminei de fazer parse dos headers do http\n");
          break;
        }

        parser->index = 0;
        parser->state = s_req_header_field;
        parser->header_state = s_general;

        char c = tokens[ch];
        if ( c == 0 ) {
          printf("invalid byte in header token '%c' '/'\n", ch);
          return BROOK_ERROR;
        }

        // ... set state from first letter of header ...
        switch (c) {
          case 'c':
            parser->header_state = s_C;
            break;
          default:
            parser->header_state = s_general;
            break;
        }

        break;
      }

      case s_req_header_field:
      {
        // ... is the end of header ':' ...
        if ( ch == ':' ) {
          parser->state = s_req_header_value_start;
          printf("encontrei um header token\n");
          break;
        }

        // ... parse all header token ...
        char c = tokens[ch];
        if ( c == 0 ) {
          printf("invalid byte in header token '%c' '/'\n", ch);
          return BROOK_ERROR;
        }

        // ... testing matches to header ...
        if ( parser->header_state == s_C && c == 'o' ) {
          parser->header_state = s_CO;
          ++parser->index;
        } else if ( parser->header_state == s_CO && c == 'n' ) {
          parser->header_state = s_CON;
          ++parser->index;
        } else if ( parser->header_state == s_CON && c == 't' ) {
          parser->header_state = s_CONTENT;
          ++parser->index;
        }else if ( parser->header_state == s_CONTENT ) {
          ++parser->index;
          if ( parser->index <= sizeof(CONTENT_LENGTH) - 1 && c == CONTENT_LENGTH[parser->index] ) {
            // ... for now is content length, keep going
            if ( parser->index == (sizeof(CONTENT_LENGTH) - 2) ) {
              parser->header_state = s_content_length;
            }
          } else if ( parser->index <= sizeof(CONTENT_TYPE) - 1 && c == CONTENT_TYPE[parser->index] ) {
            // ... for now is content length, keep going
            if ( parser->index == (sizeof(CONTENT_TYPE) - 2) ) {
              parser->header_state = s_content_type;
            }
          } else {
            parser->header_state = s_general;
          }
        } else {
          parser->header_state = s_general;
        }

        break;
      }

      case s_req_header_value_start:
      {
        if (ch == ' ' || ch == '\t') {
          break;
        }
        parser->state = s_req_header_value;
        if ( parser->header_state == s_content_length ) {
          parser->content_length = 0;
        }
      }

      case s_req_header_value:
      {
        if ( ch == '\r' ) {
          parser->index = 1;
          break;
        } else if ( ch == '\n' ) {
          if ( parser->index == 1 ) {
            parser->state = s_req_header_field_start;
            parser->index = 0;
            break;
          } else {
            printf("Barra N '||n' e invalido no meio do valor \n");
            return BROOK_ERROR;
          }
        }

        // ... parsing value of Content-Length ...
        if ( parser->header_state == s_content_length ) {
          if ( !IS_NUM(ch) ) {
            printf("o valor do content length nao e numero\n");
            return BROOK_ERROR;
          }
          uint64_t t = parser->content_length;
          t *= 10;
          t += ch - '0';
          parser->content_length = t;
        }
        break;
      }

      case s_req_headers_done:
      case s_req_body:
      {
        // ... for now we dont do nothing in body, just ignore
        // ... only count the read body bytes if necessary
        ++parser->nread;
        break;
      }

      default: break;
    }

  }

  return BROOK_OK;
}

int
brook_http_parse_method () {


  return BROOK_OK;
}
