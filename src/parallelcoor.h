//
// C++ Interface: parallelcoor
//
// Description: Holds ParallelCoorView & ParallelCoorDataSet
//
//		The main idea here is that a classification can generally
// 		be expressed as multidimensional space. So we dumbly follow
//		what has been said to be an acceptable vizualisation method
// 		for this type of data set.
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef PARALLELCOOR_H
#define PARALLELCOOR_H

#include <QString>
#include <QPair>
#include <QMap>
#include <QList>
#include <QGraphicsView>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>


/**
	TODO describe _precisely_ how datas are stored
*/
typedef QList< QList<int> > ParallelCoorDataType;
class ParallelCoorDataSet : public QMap<QString, QList<QString> > 
{
	public:
		static const QString FieldSep;
		
		ParallelCoorDataSet();
		~ParallelCoorDataSet();
		
		// get Field of a complete "path"
		QString getField(const QString& p){return p.mid(0,p.indexOf(FieldSep)-1);}
		// get Place of a complete "path"
		QString getPlace(const QString& p){return p.right(p.indexOf(FieldSep));}

	private:
		// descriptions of "dimensions"
		QMap<QString, QString> m_categoryDescriptions;

		// descriptions of "places". keys are dimension + FieldSep + place
		QMap<QString, QString> m_valueDescriptions;

		ParallelCoorDataType m_data;

	public:
		// put here set/get methods
		void setCategoryDescriptions ( const QMap< QString , QString >& theValue );
		QMap< QString , QString> getCategoryDescriptions() const;
		void setValueDescriptions ( const QMap< QString , QString >& theValue );
		QMap< QString , QString> getValueDescriptions() const;
		void setData ( const ParallelCoorDataType& theValue );
		ParallelCoorDataType getData() const;

		/// compatibility layer :)
		QPair<QString, QList<QString> > at(int idx)
		{
			if(idx < count())
			{
				const QString k(keys().at(idx));
				return QPair<QString, QList<QString> >(k, value(k));
			}
			return QPair<QString, QList<QString> >();
		}

};


class ParallelCoorFieldItem : public QGraphicsSimpleTextItem
{
	public:
		ParallelCoorFieldItem(QString text, QGraphicsView* pcv, QGraphicsItem * parent = 0);
		~ParallelCoorFieldItem(){}
		
	protected:
		void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
		void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
		void mousePressEvent ( QGraphicsSceneMouseEvent * event );
		void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
		
	private:
		QGraphicsView* pview;
		
};

class ParallelCoorValueItem : public QGraphicsSimpleTextItem
{
	public:
		ParallelCoorValueItem(QString text, QGraphicsView* pcv, QGraphicsItem * parent = 0);
		~ParallelCoorValueItem(){}
		
		void hoverEnter();
		void hoverLeave();
		void click(int mod = 0);
		
	protected:
		void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
		void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
		void mousePressEvent ( QGraphicsSceneMouseEvent * event );
		void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
		
	private:
		QGraphicsView* pview;
};

class ParallelCoorBarItem : public QGraphicsLineItem
{
	public:
		ParallelCoorBarItem(const QString& field, QGraphicsView* pcv, QGraphicsItem * parent = 0);
		~ParallelCoorBarItem(){}	
		
	protected:
		void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
		void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
		void mousePressEvent ( QGraphicsSceneMouseEvent * event );
		void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
	private:
		QGraphicsView* pview;
		QString attachedField;
};

class ParallelCoorMarkItem : public QGraphicsPathItem
{
	public:
		ParallelCoorMarkItem(ParallelCoorValueItem* relative, QGraphicsView* pcv, QGraphicsItem * parent = 0);
		~ParallelCoorMarkItem(){}
		
	protected:
		void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
		void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
		void mousePressEvent ( QGraphicsSceneMouseEvent * event );
		void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
		
	private:
		QGraphicsView * pview;
		ParallelCoorValueItem * value;
};


class ParallelCoorView : public QGraphicsView
{
	Q_OBJECT
	public:
		ParallelCoorView ( QWidget * parent = 0 );
		ParallelCoorView ( ParallelCoorDataSet * dataset, QWidget * parent = 0 );
		~ParallelCoorView();
		
		void selectField(const QString& field);
		bool matchFilter(QList<int> list) const;
	
		// put here set/get methods
		void setDataSet ( ParallelCoorDataSet* theValue );
		ParallelCoorDataSet* getDataSet() const;
		void setFilter ( const QMap< QString, QStringList >& theValue );
		QMap< QString, QStringList > getFilter() const;
		QString filterAsString();
		void setCurrentField ( const QString& theValue );
		QString getCurrentField() const;
		
	public slots:
		void updateGraphic();

	protected:
		// main method that redraws all
		virtual void redraw();
		
		void resizeEvent ( QResizeEvent * event );
		void showEvent ( QShowEvent * event );
	signals:
		void selectedField(const QString&);
		void filterChanged();

	private:
		void doConnect();
		// At some point the dataset will send signals
		// when data is updated. Thus, beeing a descendant of
		// QObject, it will "inherit" the Q_DISABLE_COPY macro.
		ParallelCoorDataSet * m_dataSet;
		
		// if empty, all items of the dataset are shown.
		// else, only datas that match the filter
		QMap<QString, QStringList> m_filter;
		QMap<int, QList<int> > cfilter; // something like a compiled version of the filter
		
		// hm, not very meaningful but it can be useful.
		QString m_currentField;
		
		// the very privates with no access methods.
		QList<ParallelCoorValueItem*> valueLabels;
		QList<ParallelCoorFieldItem*> fieldLabels;
		QList<QGraphicsLineItem*> vertices;
		QList<ParallelCoorBarItem*> bars;
		QList<ParallelCoorMarkItem*> marks;
		
		struct Units
		{
			double hunit;
			double wunit;
			double XOffset;
			double YOffset;
			double H;
			double W ;
			int C ;
			double step ;
			Units() {}
			Units (int width, int height, int count );
		};
		
		Units units;
		enum ItemList{AllList, BarList, VerticeList, FieldList, ValueList};
		
		
		void initPensAndBrushes();
		
		void cleanLists(ItemList il = AllList);
		void drawBars();
		void drawVertices();
		void drawFields();
		void drawValues();
		
		QSize controlSize;
		
	private slots:
		void slotSaveColors();
		
	public:
		static QMap<QString, QPen> pens;
		static QMap<QString, QBrush> brushes;
		static QPainterPath markPath;
		

};

#endif
