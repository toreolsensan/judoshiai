include ../common/Makefile.inc

ifeq ($(OS),Windows_NT)
DOCPATH=$(shell pwd -W | tr '/' '\\')\\
else
DOCPATH=$(shell pwd)/
endif

DOCSHIAI_FI=judoshiai-fi.pdf
DOCSHIAI_EN=judoshiai-en.pdf
DOCTIMER_FI=judotimer-fi.pdf
DOCTIMER_EN=judotimer-en.pdf
DOCFILES=$(DOCSHIAI_FI) $(DOCSHIAI_EN) $(DOCTIMER_FI) $(DOCTIMER_EN)

all: $(DOCFILES)

%.pdf: %.odt
	rm -f $@
	soffice "macro:///Standard.Module1.ConvertWordToPDF($(DOCPATH)$<)"
	while test ! \( -e $@ \) ; do \
	echo "Waiting for $@"; sleep 1; done

%.pdf: %.odg
	rm -f $@
	soffice "macro:///Standard.Module1.ConvertWordToPDF($(DOCPATH)$<)"
	while test ! \( -e $@ \) ; do \
	echo "Waiting for $@"; sleep 1; done

clean:
	rm -f *.pdf

#####################################################################
# OpenOffice writer needs the macro                                 #
# Standard.Module1.ConvertWordToPDF                                 #
# to convert odt files to pdf files.                                #
# (http://www.oooforum.org/forum/viewtopic.phtml?t=3772)            #
#####################################################################
#
#Sub ConvertWordToPDF( cFile )
#   cURL = ConvertToURL( cFile )
#   
#   ' Open the document.
#   ' Just blindly assume that the document is of a type that OOo will
#   '  correctly recognize and open -- without specifying an import filter.
#   oDoc = StarDesktop.loadComponentFromURL( cURL, "_blank", 0, Array(_
#            MakePropertyValue( "Hidden", True ),_
#            ) )
#
#
#   cFile = Left( cFile, Len( cFile ) - 4 ) + ".pdf"
#   cURL = ConvertToURL( cFile )
#   
#   ' Save the document using a filter.   
#   oDoc.storeToURL( cURL, Array(_
#            MakePropertyValue( "FilterName", "writer_pdf_Export" ),_
#            )
#   
#   oDoc.close( True )
#End Sub
#
#
#Function MakePropertyValue( Optional cName As String, Optional uValue ) As com.sun.star.beans.PropertyValue
#   Dim oPropertyValue As New com.sun.star.beans.PropertyValue
#   If Not IsMissing( cName ) Then
#      oPropertyValue.Name = cName
#   EndIf
#   If Not IsMissing( uValue ) Then
#      oPropertyValue.Value = uValue
#   EndIf
#   MakePropertyValue() = oPropertyValue
#End Function

