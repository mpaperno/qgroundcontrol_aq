DEPENDPATH += src/libs/nmea src/libs/nmea/include
INCLUDEPATH += src/libs/nmea/include

# Input
HEADERS +=  src/libs/nmea/include/nmea/config.h \
            src/libs/nmea/include/nmea/context.h \
            src/libs/nmea/include/nmea/gmath.h \
            src/libs/nmea/include/nmea/info.h \
            src/libs/nmea/include/nmea/nmea.h \
            src/libs/nmea/include/nmea/parse.h \
            src/libs/nmea/include/nmea/parser.h \
            src/libs/nmea/include/nmea/sentence.h \
            src/libs/nmea/include/nmea/time.h \
            src/libs/nmea/include/nmea/tok.h \
            src/libs/nmea/include/nmea/units.h
SOURCES +=  src/libs/nmea/context.c \
           src/libs/nmea/gmath.c \
            src/libs/nmea/info.c \
            src/libs/nmea/parse.c \
            src/libs/nmea/parser.c \
            src/libs/nmea/sentence.c \
            src/libs/nmea/time.c \
            src/libs/nmea/tok.c
