//
// C++ Implementation: dataexport
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "dataexport.h"
#include "ui_dataexport.h"

#include "typotek.h"
#include "fontitem.h"
#include "fmfontdb.h"
#include "fminfodisplay.h"

#include <QFile>
#include <QXmlStreamWriter>
#include <QDebug>
#include <QProgressDialog>
#include <QListWidgetItem>
#include <QFileDialog>

//DataExport::DataExport(const QString &dirPath, const QString &filterTag)
//{
//	exDir.setPath(dirPath);
//	filter = filterTag;
//	fonts = FMFontDb::DB()->Fonts(filter,FMFontDb::Tags);
//}

DataExport::DataExport(QWidget* parent):
		QWidget(parent,Qt::Window),
		ui(new Ui::DataExport)
{
	setAttribute(Qt::WA_DeleteOnClose, true);
	ui->setupUi(this);
	fonts = FMFontDb::DB()->getFilteredFonts();
	foreach(FontItem* f, fonts)
	{
		QListWidgetItem *it(new QListWidgetItem(f->path()));
		it->setCheckState(Qt::Checked);
		ui->listWidget->addItem(it);
	}

	show();
	connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->continueButton, SIGNAL(clicked()), this, SLOT(doExport()));
}

DataExport::~DataExport()
{

}

void DataExport::doExport()
{
	QString dir( QDir::homePath() );
	dir = QFileDialog::getExistingDirectory ( this, tr ( "Choose Directory" ), dir  ,  QFileDialog::ShowDirsOnly );
	if ( dir.isEmpty() )
		return ;
	exDir = QDir(dir);

	copyFiles();
	buildHtml();
	close();
}


int DataExport::copyFiles()
{
	QProgressDialog progress ( QObject::tr ( "Copying files" ), QObject::tr ( "cancel" ), 0, fonts.count(), this );
	progress.setWindowModality ( Qt::WindowModal );
	int progressindex(0);
	QList<int> toRemove;
	for(int fidx( 0 ); fidx < fonts.count() ; ++fidx)
	{
		if ( progress.wasCanceled() )
			break;

		progress.setLabelText ( fonts[fidx]->fancyName() );
		progress.setValue ( ++progressindex );
		
		QFile ffile(fonts[fidx]->path());
		QFileInfo ifile(ffile);
		if(ffile.copy(exDir.absolutePath() + exDir.separator() + ifile.fileName()) )
		{
			if ( !fonts[fidx]->afm().isEmpty() )
			{
				if ( !QFile::copy( fonts[fidx]->afm(), exDir.absolutePath() + exDir.separator() +  fonts[fidx]->activationAFMName() ) )
				{
					qDebug() << "unable to copy " << fonts[fidx]->afm();
				}
				else
				{
					qDebug() << fonts[fidx]->afm() << "copied";
				}
			}
		}
		else
			toRemove << fidx;
	}
	
	for(int i(toRemove.count() - 1); i >= 0;--i)
		fonts.removeAt(toRemove[i]);
	return 0;
}

int DataExport::buildIndex()
{
	QFile file(exDir.absolutePath() + exDir.separator() +"fontmatrix.data");
	QXmlStreamWriter xmlStream(&file);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qDebug() << "Export Warning : Can't open " << file.fileName();
		return 0;
	}
	else
	{
		xmlStream.setAutoFormatting(true);
	}
	
	xmlStream.writeStartDocument();
	xmlStream.writeStartElement("fontmatrix");
	xmlStream.writeAttribute("version", "1.1");
	
	for(int fidx( 0 ); fidx < fonts.count() ; ++fidx)
	{
		FontItem* fitem(fonts[fidx]);
		{
			xmlStream.writeStartElement("fontfile");
			xmlStream.writeAttribute("family", fitem->family());
			xmlStream.writeAttribute("variant",fitem->variant());
			xmlStream.writeAttribute("type",fitem->type());
			xmlStream.writeStartElement("file");
			xmlStream.writeCharacters( QFileInfo(fitem->path()).fileName() );
			xmlStream.writeEndElement();
			xmlStream.writeStartElement("info");
			FMInfoDisplay fid(fitem);
			xmlStream.writeCharacters( fid.getHtml() );
			xmlStream.writeEndElement();
			QStringList tl = fitem->tags();
			// 			tl.removeAll("Activated_On");
			// 			tl.removeAll("Activated_Off");
			foreach(QString tag, tl)
			{
				xmlStream.writeStartElement("tag");
				xmlStream.writeCharacters( tag );
				xmlStream.writeEndElement();
			}
			xmlStream.writeEndElement();
		}		
	}
	
	xmlStream.writeEndElement();//fontmatrix
	xmlStream.writeEndDocument();
	file.close();
	return fonts.count();
}


int DataExport::buildHtml()
{
	QFile file(exDir.absolutePath() + exDir.separator() +"index.html");
	QXmlStreamWriter xmlStream(&file);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qDebug() << "Export Warning : Can't open " << file.fileName();
		return 0;
	}
	else
	{
		xmlStream.setAutoFormatting(true);
	}
	
	xmlStream.writeStartDocument();
	xmlStream.writeStartElement("html");
	
	// header
	xmlStream.writeStartElement("head");
	xmlStream.writeStartElement("title");
	xmlStream.writeCharacters( "Fontmatrix - " + filter);
	xmlStream.writeEndElement();
	xmlStream.writeEndElement();
	
	xmlStream.writeStartElement("body");
	for(int fidx( 0 ); fidx < fonts.count() ; ++fidx)
	{
		FontItem* fitem(fonts[fidx]);
		{
			QFileInfo ffile(fitem->path());
			
			xmlStream.writeStartElement("div");
			xmlStream.writeAttribute("class", "fontbox");
			
			xmlStream.writeStartElement("div");
			xmlStream.writeAttribute("class", "namebox");
			
			xmlStream.writeStartElement("a");
			xmlStream.writeAttribute("href", ffile.fileName() );
			xmlStream.writeCharacters( fitem->fancyName() );
			xmlStream.writeEndElement();// a
			
			xmlStream.writeEndElement();// div.namebox
			
			//			xmlStream.writeStartElement("img");
			//			xmlStream.writeAttribute("class", "imgbox");
			//			xmlStream.writeAttribute("src", ffile.fileName() + ".png");
			//			xmlStream.writeEndElement();// img.imgbox
			
			xmlStream.writeStartElement("div");
			xmlStream.writeAttribute("class", "infobox");
			
			QStringList tl = fitem->tags();
			// 			tl.removeAll("Activated_On");
			// 			tl.removeAll("Activated_Off");
			foreach(QString tag, tl)
			{
				xmlStream.writeStartElement("div");
				xmlStream.writeAttribute("class", "tagbox");
				xmlStream.writeCharacters( tag );
				xmlStream.writeEndElement(); //div.tagbox
			}
			xmlStream.writeEndElement();// div.info
			xmlStream.writeEndElement();// div.fontbox
		}		
	}
	xmlStream.writeEndElement();// body
	xmlStream.writeEndElement();// html
	xmlStream.writeEndDocument();
	file.close();
	return fonts.count();
}

int DataExport::buildTemplate(const QString& templateDirPath)
{
	/// A template is the addition of 3 files in a directory
	/// Which are : TOP CENTER BOTTOM
	/// In TOP and BOTTOM, just put what you want
	/// In CENTER, these strings will be replaced:
	// ##FILENAME##
	// ##FAMILY##
	// ##VARIANT##
	// ##PREVIEW##
	// ##TAGS##
	
	QFile file(exDir.absolutePath() + exDir.separator() +"export.html");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qDebug() << "Export Warning : Can't open " << file.fileName();
		return 0;
	}
	
	QDir tDir(templateDirPath);
	if(!tDir.exists("TOP"))
		return 0;
	if(!tDir.exists("CENTER"))
		return 0;
	if(!tDir.exists("BOTTOM"))
		return 0;
	
	
	QFile fileTOP(tDir.filePath("TOP"));
	QFile fileCENTER(tDir.filePath("CENTER"));
	QFile fileBOTTOM(tDir.filePath("BOTTOM"));
	
	if(!fileTOP.open(QIODevice::ReadOnly | QIODevice::Text))
		return 0;
	if(!fileCENTER.open(QIODevice::ReadOnly | QIODevice::Text))
		return 0;
	if(!fileBOTTOM.open(QIODevice::ReadOnly | QIODevice::Text))
		return 0;
	
	QString sTOP(fileTOP.readAll());
	fileTOP.close();
	QString sCENTER(fileCENTER.readAll());
	fileCENTER.close();
	QString sBOTTOM(fileBOTTOM.readAll());
	fileBOTTOM.close();
	
	QTextStream exp(&file);
	
	exp << sTOP;
	for(int fidx( 0 ); fidx < fonts.count() ; ++fidx)
	{
		FontItem* fitem(fonts[fidx]);
		{
			QString t(sCENTER);
			QFileInfo ffile(fitem->path());
			
			t.replace("##FILENAME##", ffile.fileName() );
			t.replace("##PREVIEW##",  ffile.fileName()+".png");
			t.replace("##FAMILY##", fitem->family() );
			t.replace("##VARIANT##", fitem->variant() );
			t.replace("##TAGS##", fitem->tags().join(", "));
			
			exp << t;
		}
	}
	exp << sBOTTOM;
	
	file.close();	
	return fonts.count();
}


