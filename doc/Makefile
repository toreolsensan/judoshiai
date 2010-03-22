include ../common/Makefile.inc

ifeq ($(OS),Windows_NT)
ODTFILE=$(shell pwd | tr '/' '\\' | sed 's/\\c\\/C:\\/')\judoshiai.odt
ODTFILE2=$(shell pwd | tr '/' '\\' | sed 's/\\c\\/C:\\/')\judotimer.odg
else
ODTFILE=$(PWD)/doc/judoshiai.odt
ODTFILE2=$(PWD)/doc/judotimer.odg
endif

all: judoshiai.pdf judotimer.pdf

judoshiai.pdf: judoshiai.odt
	soffice "macro:///Standard.Module1.ConvertWordToPDF($(ODTFILE))"

judotimer.pdf: judotimer.odg
	soffice "macro:///Standard.Module1.ConvertWordToPDF($(ODTFILE2))"

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

