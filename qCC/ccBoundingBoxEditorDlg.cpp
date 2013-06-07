//##########################################################################
//#                                                                        #
//#                            CLOUDCOMPARE                                #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 of the License.               #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#          COPYRIGHT: EDF R&D / TELECOM ParisTech (ENST-TSI)             #
//#                                                                        #
//##########################################################################

#include "ccBoundingBoxEditorDlg.h"

ccBoundingBoxEditorDlg::ccBoundingBoxEditorDlg(QWidget* parent/*=0*/)
	: QDialog(parent)
	, Ui::BoundingBoxEditorDialog()
{
    setupUi(this);

    setWindowFlags(Qt::Tool);

	xDoubleSpinBox->setMinimum(-DBL_MAX);
	yDoubleSpinBox->setMinimum(-DBL_MAX);
	zDoubleSpinBox->setMinimum(-DBL_MAX);
	xDoubleSpinBox->setMaximum(DBL_MAX);
	yDoubleSpinBox->setMaximum(DBL_MAX);
	zDoubleSpinBox->setMaximum(DBL_MAX);

	dxDoubleSpinBox->setMinimum(0.0);
	dyDoubleSpinBox->setMinimum(0.0);
	dzDoubleSpinBox->setMinimum(0.0);
	dxDoubleSpinBox->setMaximum(DBL_MAX);
	dyDoubleSpinBox->setMaximum(DBL_MAX);
	dzDoubleSpinBox->setMaximum(DBL_MAX);

	connect(pointTypeComboBox,	SIGNAL(currentIndexChanged(int)),	this,	SLOT(reflectChanges(int)));
	connect(keepSquareCheckBox,	SIGNAL(toggled(bool)),				this,	SLOT(squareModeActivated(bool)));
	connect(okPushButton,		SIGNAL(clicked()),					this,	SLOT(accept()));
	connect(cancelPushButton,	SIGNAL(clicked()),					this,	SLOT(cancel()));
	connect(resetPushButton,	SIGNAL(clicked()),					this,	SLOT(reset()));

	connect(xDoubleSpinBox,		SIGNAL(valueChanged(double)),		this,	SLOT(updateCurrentBBox(double)));	
	connect(yDoubleSpinBox,		SIGNAL(valueChanged(double)),		this,	SLOT(updateCurrentBBox(double)));	
	connect(zDoubleSpinBox,		SIGNAL(valueChanged(double)),		this,	SLOT(updateCurrentBBox(double)));	

	connect(dxDoubleSpinBox,	SIGNAL(valueChanged(double)),		this,	SLOT(updateXWidth(double)));	
	connect(dyDoubleSpinBox,	SIGNAL(valueChanged(double)),		this,	SLOT(updateYWidth(double)));	
	connect(dzDoubleSpinBox,	SIGNAL(valueChanged(double)),		this,	SLOT(updateZWidth(double)));	

	resetPushButton->setVisible(false);
	checkBaseInclusion();
}

//Helper
void MakeSquare(ccBBox& box, int pivotType, int defaultDim = -1)
{
	assert(defaultDim<3);
	assert(pivotType>=0 && pivotType<3);

	CCVector3 W = box.getDiagVec();
	if (W.x != W.y || W.x != W.z)
	{
		if (defaultDim < 0)
		{
			//we take the largest one!
			defaultDim = 0;
			if (W.u[1]>W.u[defaultDim])
				defaultDim = 1;
			if (W.u[2]>W.u[defaultDim])
				defaultDim = 2;
		}

		CCVector3 newW(W.u[defaultDim],W.u[defaultDim],W.u[defaultDim]);
		switch(pivotType)
		{
		case 0: //min corner
			{
				CCVector3 A = box.minCorner();
				box = ccBBox(A,A+newW);
			}
			break;
		case 1: //center
			{
				CCVector3 C = box.getCenter();
				box = ccBBox(C-newW/2.0,C+newW/2.0);
			}
			break;
		case 2: //max corner
			{
				CCVector3 B = box.maxCorner();
				box = ccBBox(B-newW,B);
			}
			break;
		}
	}
}

bool ccBoundingBoxEditorDlg::keepSquare() const
{
	return keepSquareCheckBox->isChecked();
}

void ccBoundingBoxEditorDlg::forceKeepSquare(bool state)
{
	if (state)
		keepSquareCheckBox->setChecked(true);
	keepSquareCheckBox->setDisabled(state);
}

void ccBoundingBoxEditorDlg::squareModeActivated(bool state)
{
	if (state)
	{
		MakeSquare(m_currentBBox,pointTypeComboBox->currentIndex());
		reflectChanges();
	}
}

void ccBoundingBoxEditorDlg::set2DMode(bool state)
{
	zDoubleSpinBox->setVisible(state);
	dzDoubleSpinBox->setVisible(state);
	zLabel->setVisible(state);
}

void ccBoundingBoxEditorDlg::setBaseBBox(const ccBBox& box)
{
	m_initBBox = m_baseBBox = box;

	resetPushButton->setVisible(m_baseBBox.isValid());

	reset();
}

void ccBoundingBoxEditorDlg::checkBaseInclusion()
{
	bool exclude = false;
	if (m_baseBBox.isValid())
	{
		exclude = !m_currentBBox.inside(m_baseBBox.minCorner()) || !m_currentBBox.inside(m_baseBBox.maxCorner());
	}

	warningLabel->setVisible(exclude);
	okPushButton->setEnabled(!exclude);
}

void ccBoundingBoxEditorDlg::reset()
{
	m_currentBBox = m_baseBBox;
	
	if (keepSquare())
		squareModeActivated(true); //will call reflectChanges
	else
		reflectChanges();

	checkBaseInclusion();
}

int	ccBoundingBoxEditorDlg::exec()
{
	//backup current box
	m_initBBox = m_currentBBox;

	//call 'true' exec
	return QDialog::exec();
}

void ccBoundingBoxEditorDlg::cancel()
{
	//restore init. box
	m_currentBBox = m_initBBox;

	reject();
}

void ccBoundingBoxEditorDlg::updateXWidth(double value)
{
	updateCurrentBBox(value);
	if (keepSquare())
	{
		MakeSquare(m_currentBBox,pointTypeComboBox->currentIndex(),0);
		reflectChanges();
		//base box (if valid) should always be included!
		if (m_baseBBox.isValid())
			checkBaseInclusion();
	}
}

void ccBoundingBoxEditorDlg::updateYWidth(double value)
{
	updateCurrentBBox(value);
	if (keepSquare())
	{
		MakeSquare(m_currentBBox,pointTypeComboBox->currentIndex(),1);
		reflectChanges();
		//base box (if valid) should always be included!
		if (m_baseBBox.isValid())
			checkBaseInclusion();
	}
}

void ccBoundingBoxEditorDlg::updateZWidth(double value)
{
	updateCurrentBBox(value);
	if (keepSquare())
	{
		MakeSquare(m_currentBBox,pointTypeComboBox->currentIndex(),2);
		reflectChanges();
		//base box (if valid) should always be included!
		if (m_baseBBox.isValid())
			checkBaseInclusion();
	}
}

void ccBoundingBoxEditorDlg::updateCurrentBBox(double dummy)
{
	CCVector3 A(xDoubleSpinBox->value(),
				yDoubleSpinBox->value(),
				zDoubleSpinBox->value());
	CCVector3 W(dxDoubleSpinBox->value(),
				dyDoubleSpinBox->value(),
				dzDoubleSpinBox->value());

	switch (pointTypeComboBox->currentIndex())
	{
	case 0: //A = min corner
		m_currentBBox = ccBBox(A,A+W);
		break;
	case 1: //A = center
		m_currentBBox = ccBBox(A-W/2.0,A+W/2.0);
		break;
	case 2: //A = max corner
		m_currentBBox = ccBBox(A-W,A);
		break;
	default:
		assert(false);
		return;
	}

	//base box (if valid) should always be included!
	if (m_baseBBox.isValid())
		checkBaseInclusion();
}

void ccBoundingBoxEditorDlg::reflectChanges(int dummy)
{
	//left column
	{
		xDoubleSpinBox->blockSignals(true);
		yDoubleSpinBox->blockSignals(true);
		zDoubleSpinBox->blockSignals(true);

		switch (pointTypeComboBox->currentIndex())
		{
		case 0: //A = min corner
			{
				const CCVector3& A = m_currentBBox.minCorner();
				xDoubleSpinBox->setValue(A.x);
				yDoubleSpinBox->setValue(A.y);
				zDoubleSpinBox->setValue(A.z);
			}
			break;
		case 1: //A = center
			{
				CCVector3 C = m_currentBBox.getCenter();
				xDoubleSpinBox->setValue(C.x);
				yDoubleSpinBox->setValue(C.y);
				zDoubleSpinBox->setValue(C.z);
			}
			break;
		case 2: //A = max corner
			{
				const CCVector3& B = m_currentBBox.maxCorner();
				xDoubleSpinBox->setValue(B.x);
				yDoubleSpinBox->setValue(B.y);
				zDoubleSpinBox->setValue(B.z);
			}
			break;
		default:
			assert(false);
			return;
		}

		xDoubleSpinBox->blockSignals(false);
		yDoubleSpinBox->blockSignals(false);
		zDoubleSpinBox->blockSignals(false);
	}

	//right column
	{
		dxDoubleSpinBox->blockSignals(true);
		dyDoubleSpinBox->blockSignals(true);
		dzDoubleSpinBox->blockSignals(true);

		CCVector3 W = m_currentBBox.getDiagVec();
		//if 'square mode' is on, all width values should be the same!
		assert(!keepSquare() || W.x == W.y && W.x == W.z);
		dxDoubleSpinBox->setValue(W.x);
		dyDoubleSpinBox->setValue(W.y);
		dzDoubleSpinBox->setValue(W.z);

		dxDoubleSpinBox->blockSignals(false);
		dyDoubleSpinBox->blockSignals(false);
		dzDoubleSpinBox->blockSignals(false);
	}
}
