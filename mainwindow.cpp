#include "mainwindow.h"
#include "ui_simUVWidget.h"

#include <QDateTimeEdit>
#include <QFileDialog>
#include <QtGui>
#include <QList>
#include <QListWidgetItem>
#include <QListWidget>
#include <QSharedPointer>

#include <meshlab/mainwindow.h>
#include <wrap/gl/trimesh.h>
#include <meshlab/stdpardialog.h>

#include "OldAPISupport.h"
#include "SpreadSheet.h"
#include "Helpers.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
	ui = new Ui::MainWindow;
	ui->setupUi(this);
	this->setCentralWidget(ui->main_frame);

	spreadsheet = new SpreadSheet();
	MeshDocument *mydoc;
	meshDoc = mydoc;

	zones = 0;
	poi = 0;
	minDate = 0;
	maxDate = 0;
	anatZonesChanged = false;
	protectionsLibChanged = false;
	protectionsChanged = false;
	currentAnatFilename = "";
	currentProtectionsLibFilename = "";
	currentProtectionsFilename = "";
	numPositions = 0;
	sources = 0;


	// Directories
//	updateExportPaths();
//	connect(ui->checkBoxSubdirectory, SIGNAL(toggled(bool)), this, SLOT(updateExportPaths()));
//	connect(ui->lineEditSubdirectory, SIGNAL(textChanged(QString)), this, SLOT(updateExportPaths()));
//	ui->diffuseMapLineEdit->setText(meshDoc->mm()->pathName()+"/%position%-diffuseMap.txt");
//	ui->reflectedMapLineEdit->setText(meshDoc->mm()->pathName()+"/%position%-reflectedMap");

//	connect(ui->loadWeatherDataButton, SIGNAL(clicked()),this,SLOT(loadWeatherDataFile()));
}

MainWindow::~MainWindow()
{

	Helpers::deleteZones(&zones);
	if(poi){
		delete poi;
	}
	if(maxDate){
		delete maxDate;
	}
	if(minDate){
		delete minDate;
	}

	delete spreadsheet;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	emit closing();
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
	bool raiseRequired = false;
	if(event->type() == QEvent::WindowStateChange)
	{
		auto window = dynamic_cast<QMainWindow *>(object);
		if(window && window->windowState() != Qt::WindowMinimized)
			raiseRequired = true;
	}
	else if(event->type() == QEvent::ActivationChange)
	{
		auto window = dynamic_cast<QMainWindow *>(object);
		if(window)
			raiseRequired = true;
	}
	if(raiseRequired)
		raise();
	return false;
}

//General
void MainWindow::loadWeatherDataFile(){
	//Get file to open
	QString dir;
	if (lastDirectory == ""){
		dir=".";
	}else{
		dir = lastDirectory;
	}
	QString filename = QFileDialog::getOpenFileName(this,tr("Select Weather Data File"),dir, spreadsheet->extensions().join(";;"));
	if(!filename.isNull()){
		
		//Open the Excel data file
		if (spreadsheet->init(filename.toStdString().c_str())!=0 || spreadsheet->open(filename.toStdString().c_str())!=0){
			QMessageBox::information(0, "Failed to open Excel File.", "Error");
			return;
		}
		
		//Check Excel Data File Format
		if(!spreadsheet->getFirstDate() || !spreadsheet->getLastDate() || spreadsheet->getTimeStep()<1){
			QMessageBox::information(0, "Excel file does not seem to be in the right format. Please check accepted format.", "Error");
			return;
		}

		//Store the dates
		minDate = new QDateTime(QDate(spreadsheet->getFirstDate()->getYear(),spreadsheet->getFirstDate()->getMonth(),spreadsheet->getFirstDate()->getDay()),QTime(spreadsheet->getFirstDate()->getHour(),spreadsheet->getFirstDate()->getMinute()));
		maxDate = new QDateTime(QDate(spreadsheet->getLastDate()->getYear(),spreadsheet->getLastDate()->getMonth(),spreadsheet->getLastDate()->getDay()),QTime(spreadsheet->getLastDate()->getHour(),spreadsheet->getLastDate()->getMinute()));

		//Setup GUI based on Excel file information
		weatherDataLoaded = true;
		QFileInfo finfo(filename);
		lastDirectory = finfo.absolutePath();
		ui->weatherDataFileLineEdit->setText(filename);

		//Refresh positions tab
		reloadPositionsTab();		
		
	}
}

//Result file names handling
void MainWindow::exportVertCSVFile(){
	exportDirBrowse("Select Directory to Export Vertex Intensities", ui->exportVertCSVLineEdit);
}

void MainWindow::exportControlCSVFile(){
	exportDirBrowse("Select Directory to Export Control Surface Intensities", ui->exportFlatCSVLineEdit);
}

void MainWindow::exportZonesCSVFile(){
	exportDirBrowse("Select Directory to Export Zones Intensities", ui->exportAnatZonesLineEdit);
}

void MainWindow::exportPOICSVFile(){
	exportDirBrowse("Select Directory to Export POI Intensities", ui->exportPOILineEdit);
}

void MainWindow::exportProtectionsFile(){
	exportDirBrowse("Select Directory to Export Protections File", ui->exportProtectionsLineEdit);
}

void MainWindow::exportDiffuseMapFile(){
	exportDirBrowse("Select Directory to Export Diffuse Map", ui->diffuseMapLineEdit);
}

void MainWindow::exportReflectedMapFile(){
	exportDirBrowse("Select Directory to Export Reflected Map", ui->reflectedMapLineEdit);
}

void MainWindow::filePositionLineEditChanged(){
	
	QFileInfo finfo(((QLineEdit*)QObject::sender())->text());

	//Check that the dir exists
	if(!finfo.dir().exists()){
		QMessageBox::warning(this,"Directory not found","The current directory does not exist. Please select an existing directory.");
	}

	//Check that the filename is appropriate	
	//search for the %position% string
	if(finfo.fileName().lastIndexOf("%position%") < 0){
		QMessageBox::warning(this,"Not a generic name", "The current filename will only work for a single position simulation. Valid filenames for multiple positions simulation should include the pattern '%position%' which will be replaced by individual position names.");
	}
}

void MainWindow::fileIntensitiesLineEditChanged(){
		QFileInfo finfo(((QLineEdit*)QObject::sender())->text());

	//Check that the dir exists
	if(!finfo.dir().exists()){
		QMessageBox::warning(this,"Directory not found","The current directory does not exist. Please select an existing directory.");
	}

	//Check that the filename is appropriate	
	//search for the %position% string
	if(finfo.fileName().lastIndexOf("%intensities%") < 0){
		QMessageBox::warning(this,"Not a generic name", "The current filename will only work for a single protections file. Valid filenames for multiple protections files (ex: vertex intensities+anatomical zones or multiple positions) should include the pattern '%intensities%' which will be replaced by individual intensities files name.");
	}
}

//SIMULATION
void MainWindow::runSimulation(){
	//Cache position table, find used date range
	const Date &firstDate = *spreadsheet->getFirstDate();
	const Date &lastDate = *spreadsheet->getLastDate();

	Date firstUsedDate = lastDate;
	Date lastUsedDate = firstDate;

	QVector<Position> positionsTable;
	positionsTable.resize(ui->positionsTableWidget->rowCount());
	for(int i=0; i<ui->positionsTableWidget->rowCount(); ++i){
		Position &position = positionsTable[i];
		position.checked = ((QCheckBox*)ui->positionsTableWidget->cellWidget(i,PositionsTableColumns::ACTIVE))->isChecked();

		QDateTime qStartDate = ((QDateTimeEdit*)ui->positionsTableWidget->cellWidget(i, PositionsTableColumns::STARTTIME))->dateTime();
		QDateTime qEndDate = ((QDateTimeEdit*)ui->positionsTableWidget->cellWidget(i, PositionsTableColumns::ENDTIME))->dateTime();
		position.qStartDate = qStartDate;
		position.qEndDate = qEndDate;
		position.startDate = Date(qStartDate.date().year(), qStartDate.date().month(), qStartDate.date().day(), qStartDate.time().hour(), qStartDate.time().minute());
		position.endDate = Date(qEndDate.date().year(), qEndDate.date().month(), qEndDate.date().day(), qEndDate.time().hour(), qEndDate.time().minute());
		position.timeStep = ((QTimeEdit*)ui->positionsTableWidget->cellWidget(i, PositionsTableColumns::TIMESTEP))->time();

		position.meshIndex = ((QComboBox*)ui->positionsTableWidget->cellWidget(i, PositionsTableColumns::POSITION))->currentIndex();
		position.orientation = ((QComboBox*)ui->positionsTableWidget->cellWidget(i,PositionsTableColumns::ORIENTATION))->currentIndex();
		position.startAngle = ((QDoubleSpinBox*)ui->positionsTableWidget->cellWidget(i,PositionsTableColumns::STARTANGLE))->value();
		position.endAngle = ((QDoubleSpinBox*)ui->positionsTableWidget->cellWidget(i,PositionsTableColumns::ENDANGLE))->value();
		position.angleStep = ((QDoubleSpinBox*)ui->positionsTableWidget->cellWidget(i,PositionsTableColumns::ANGULARSTEP))->value();

		if(position.checked)
		{
			if(position.startDate < firstUsedDate && position.startDate >= firstDate)
				firstUsedDate = position.startDate;
			if(position.endDate > lastUsedDate && position.endDate <= lastDate)
				lastUsedDate = position.endDate;
		}
	}

	//Check output directories
	const QString exportPath = this->exportPath();
	if(!QDir(exportPath).exists())
		QDir().mkdir(exportPath);
	
	//Keeps track of the results
	typedef QSharedPointer<UVModel> UVModelPtr;
	vector<UVModelPtr> models;
try{
	//Set the sources to be simulated
	sources = 0;
	if(ui->useDirectSourceCheckBox->isChecked()){
		sources += IntensitySources::DIRECT;
	}
	if(ui->useDiffuseSourceCheckBox->isChecked()){
		sources += IntensitySources::DIFFUSED;
	}
	if(ui->useReflectedSourceCheckBox->isChecked()){
		sources += IntensitySources::REFLECTED;
	}

	//Loop over each of the positions selected for simulation
	//And load the models in memory
	//int totalSteps = 1;
	for(int i=0; i<positionsTable.size(); ++i){

		//Check that the position has been selected for simulation
		const Position &position = positionsTable[i];
		if(position.checked){
			//Get the start and end datetimes
			QDateTime startDate = position.qStartDate;
			QDateTime endDate = position.qEndDate;
			//Check that the endDate is after tne startDate
			if(startDate.secsTo(endDate) < 1){
				QMessageBox(QMessageBox::Warning,"Dates are not valid", "The start date should always be before the end date for each of the positions. Please check the dates in the positions table on line " + QString::number(i+1) + " and relaunch the simulation.", QMessageBox::Ok, this).exec();
				return;
			}
		
			//Get the current Mesh
			MeshModel *currentMesh = meshDoc->getMesh(position.meshIndex);

			//Get the filename
			std::string filename = currentMesh->fullName().toStdString();

			//Create and initiate the UVModel
			UVModelPtr uvModel(new UVModel(meshDoc, filename.c_str(), spreadsheet));
			QString positionName = currentMesh->fullName();
			positionName = positionName.left(positionName.lastIndexOf('.'));
			positionName = positionName.mid(positionName.lastIndexOf('/') + 1);
			
			//Initialize the Model. If the user cancels the operation or a problem occurs, abort the simulation)
			if(!initUVModel(uvModel.data(), positionName))
				return;

			//store the model
			models.push_back(uvModel);

			//Add the number of necessary simulation steps to the total
			int numPositionVertices = uvModel->getUVMesh()->getModelVertices()->size();
			
			if(position.orientation == PositionsOrientations::AVERAGE){
				
				//!!The calculation of numSteps should be reused since it is used in UVModel too!!
				
				float startAngle = position.startAngle;
				float endAngle = position.endAngle;
				float angleStep = position.angleStep;

				//Make sure all angles are minimal
				startAngle = fmod(startAngle, 360.0f);
				endAngle = fmod(endAngle,360.0f);
				angleStep = fmod(angleStep,360.0f);
				
				//Make sure the step is between 1 deg and 359 deg
				//In case the step is a full turn, the average will not need extra computation
				//since it will be like a fixed position
				if(angleStep){

					//If the endAngle == startAngle,
					//user probably wants 1 full turn
					if(endAngle == startAngle){
						endAngle -= angleStep;
					}

					//Get the total rotation angle
					float totalAngle;
					if(angleStep > 0){
						 totalAngle = endAngle-startAngle;
					}
					else{
						totalAngle = startAngle-endAngle;
					}
					if(totalAngle < 0){
						totalAngle = 360 + totalAngle;
					}

					//UVs need to be evaluated for each of the steps
					numPositionVertices*=1+(totalAngle / abs(angleStep));
				}
			}
		}
	}

	//Create a progress dialog
	const int firstRow = spreadsheet->searchDateRow(firstUsedDate);
	const int lastRow = spreadsheet->searchDateRow(lastUsedDate);
	int totalSteps = lastRow - firstRow + 1;
	QProgressDialog progress("Simulation...","Cancel",0,totalSteps,this);
	progress.setWindowModality(Qt::WindowModal);
	progress.setValue(0);
	progress.show();
	progress.raise(); // bring to front

	IOZones::OutputStreamMap outputStreams;

	Date date = firstUsedDate;
	int timeStep = spreadsheet->getTimeStep();
	for(int row = firstRow; row <= lastRow; row++, date += timeStep)
	{
		progress.setValue(row - firstRow);
		if(progress.wasCanceled()){
			QMessageBox cancelCheck(QMessageBox::Warning, "Cancel Simulation?", "Are you sure you want to cancel the current simulation?",
									QMessageBox::Yes | QMessageBox::No, &progress);
			if(cancelCheck.exec() == QMessageBox::Yes)
				//throw std::exception("Simulation canceled by user."); MODIFY
			//If the user does not cancel in the end, reset the progressDialog to reset the cancel flag.
			progress.reset();
		}

		//Run the simulations
		int j = 0;
		QVector<int> skippedModels;
		for(int i=0; i<positionsTable.size(); ++i){
			const Position &position = positionsTable[i];
			//Check that the position has been selected for simulation
			if(position.checked){
				//run the simulation
				if(date >= position.startDate && date <= position.endDate)
					simulatePosition(models.at(j).data(), date, date, position);
				else
					skippedModels.push_back(j);
				//increment the results counter
				++j;
			}
		}

		//Aggregate results if necessary
		bool isAggregate = false;
		std::vector<UVModelPtr> resultModels;
		// TODO(vova.y): find better way
		resultModels = models;
		// TODO(vova.y): do we need aggregation?
		/*
		if(ui->multiplePositionsSeparateRadioButton->isChecked()){
			resultModels = models;
		}
		else{
			isAggregate = true;
			int modelsCount = models.size();

			//Get the target result
			int targetIndex = ui->targetPositionComboBox->currentIndex();
			if(modelsCount-1 < targetIndex){
				return;
			}
			UVModelPtr target = models.at(targetIndex);
			target->evaluateZones();

			//Aggregate results (Keep only the target result)
			for(int i=modelsCount-1; i >= 0; --i){
				UVModelPtr currentResult = resultModels.at(i);
				if(!(currentResult == target)){
					currentResult->evaluateZones();
					target->mergeResults(currentResult.data());
					currentResult.reset();
				}
			}

			resultModels.push_back(target);

			//Divide by the number of results in case it's an average
			if(ui->multiplePositionsAvgRadioButton->isChecked()){
				target->getUVMesh()->multiplyZonesIntensities(1.0f/modelsCount);
				target->getPlaneSurface()->multiplyIntensities(1.0f/modelsCount);
			}
		}
		*/

		//Apply protections and export the results
		bool exportTotals = row == lastRow;
		j = 0;
		for(auto resIter = resultModels.begin(); resIter != resultModels.end(); resIter++, ++j){
			if(!skippedModels.isEmpty() && skippedModels.first() == j)
			{
				skippedModels.pop_front();
				continue;
			}

			QString positionName = QString::fromLatin1((*resIter)->getUVMesh()->getModelName());
			positionName = positionName.left(positionName.lastIndexOf('.'));

			if(ui->useProtectionsCheckBox->isChecked()){
				if(ui->exportProtectionsCheckBox->isChecked()){
					exportSimulationResults(resIter->data(),positionName,false,isAggregate,outputStreams,exportTotals);
					(*resIter)->setProtections();
					exportSimulationResults(resIter->data(),positionName,true,isAggregate,outputStreams,exportTotals);
				}
				else{
					(*resIter)->setProtections();
					exportSimulationResults(resIter->data(),positionName,false,isAggregate,outputStreams,exportTotals);
				}
			}
			else{
				exportSimulationResults(resIter->data(),positionName,false,isAggregate,outputStreams,exportTotals);
			}
		}
	}

	progress.setValue(progress.maximum());

	QMessageBox(QMessageBox::Information, "Simulation done!", "The simulation has ended successfuly!",QMessageBox::Ok,this).exec();
}
catch(exception e){
	//Display error for the user
	QMessageBox(QMessageBox::Warning, "Error: Simulation stopped", QString::fromLatin1(e.what()), QMessageBox::Ok, this).exec();
}

	Helpers::deleteZones(&zones);
}

void MainWindow::simulatePosition(UVModel *uvModel, const Date &startDate, const Date &endDate, const MainWindow::Position &position)
{
	uvModel->clearEvaluatedIntensities();

	//Set the POIs if selected
	if(!uvModel->hasPOIs() && ui->usePOICheckBox->isChecked())
	{
		QFileInfo POIFile(ui->usePOILineEdit->text());
		if(POIFile.exists())
			uvModel->setPOIs(POIFile.absoluteFilePath().toStdString().c_str());
	}

	//Set the Anatomical zones
	if(!uvModel->hasZones() && ui->useAnatZonesCheckBox->isChecked())
	{
		QFileInfo anatZonesFile(ui->useAnatZonesLineEdit->text());
		if(anatZonesFile.exists())
			uvModel->setZones(anatZonesFile.absoluteFilePath().toStdString().c_str());
	}

	//Launch the simulation
	float startAngle = position.startAngle;
	float angleStep, endAngle;
	int timeStep;
	switch(position.orientation){
		case PositionsOrientations::FIXED:
			uvModel->evaluateUVBetweenFixed(startDate, endDate, startAngle, sources);
			break;

		case PositionsOrientations::AVERAGE:
			endAngle = position.endAngle;
			angleStep = position.angleStep;
			uvModel->evaluateUVBetweenAvg(startDate, endDate, startAngle, endAngle, angleStep, sources);
			break;

		case PositionsOrientations::SEQUENCE:
			angleStep = position.angleStep;
			timeStep = QTime(0,0,0).secsTo(position.timeStep);
			uvModel->evaluateUVBetweenSeq(startDate, endDate, startAngle, angleStep, timeStep, sources);
			break;
	}
}


void MainWindow::exportSimulationResults(UVModel *uvModel, QString positionName, bool exportProtectionsSeparately, bool isAggregate,
										  IOZones::OutputStreamMap &outputStreams, bool exportTotals){
	
	//Get the control surface
	Intensity flatSurfaceIntensity = uvModel->getPlaneSurface()->getTotalIntensity(false);
		
	/* RAW INTENSITIES */
	if(exportTotals && !isAggregate && ui->exportVertCheckBox->isChecked()){
		QString vertFilename;
		if(exportProtectionsSeparately){
			vertFilename = ui->exportProtectionsLineEdit->text();
			QFileInfo finfo(ui->exportVertCSVLineEdit->text());
			vertFilename.replace("%intensities%",finfo.baseName());
		}
		else{
			vertFilename = ui->exportVertCSVLineEdit->text();
		}
		vertFilename.replace("%position%",positionName);
	
		uvModel->getUVMesh()->exportIntensitiesCSV(&flatSurfaceIntensity,vertFilename.toStdString());
	}

	/* ZONES */
	if(ui->useAnatZonesCheckBox->isChecked()){
		//Aggregate must evaluate their zones earlier
		//to aggregate their results
		if(!isAggregate){
			uvModel->evaluateZones();		
		}
		if(ui->exportAnatZonesCheckBox->isChecked()){
			QString anatZonesFilename;
			if(exportProtectionsSeparately){
				anatZonesFilename = ui->exportProtectionsLineEdit->text();
				QFileInfo finfo(ui->exportAnatZonesLineEdit->text());
				anatZonesFilename.replace("%intensities%",finfo.baseName());
			}
			else{
				anatZonesFilename = ui->exportAnatZonesLineEdit->text();
			}
			anatZonesFilename.replace("%position%",positionName);
			
			uvModel->getUVMesh()->exportZonesIntensitiesCSV(&flatSurfaceIntensity,anatZonesFilename.toStdString(),outputStreams);
		}
	}
	
	/* CONTROL PLANE SURFACE */
	if(ui->exportFlatCSVCheckBox->isChecked() && !exportProtectionsSeparately){
		uvModel->getPlaneSurface()->exportIntensitiesCSV(ui->exportFlatCSVLineEdit->text().replace("%position%",positionName).toStdString(), outputStreams);
	}

	/* POI */
	// TODO(vova.y): restore
	/*
	if(!isAggregate && ui->exportPOICheckBox->isChecked()){ 
		uvModel->evaluatePOI(ui->POIRadiusSpinBox->value());	
		QString POIFilename;
		if(exportProtectionsSeparately){
			POIFilename = ui->exportProtectionsLineEdit->text();
			QFileInfo finfo(ui->exportPOILineEdit->text());
			POIFilename.replace("%intensities%",finfo.baseName());
		}
		else{
			POIFilename = ui->exportPOILineEdit->text();
		}
		POIFilename.replace("%position%",positionName);
		uvModel->getUVMesh()->exportPOIIntensitiesCSV(&flatSurfaceIntensity,POIFilename.toStdString());
	}
	*/

	/* MESH */
	if(exportTotals)
	{
		uvModel->getPlaneSurface()->evaluateTotalEvaluatedIntensity(true);
		uvModel->getUVMesh()->evaluateTotalEvaluatedIntensity(true);
		Intensity flatSurfaceIntensity = uvModel->getPlaneSurface()->getTotalIntensity(true);

		//Generate the name
		QString meshName;
		QFileInfo positionFinfo(positionName);
		if(exportProtectionsSeparately){
			QFileInfo finfo(ui->exportProtectionsLineEdit->text());
			meshName = finfo.baseName();
			meshName.replace("%intensities%",positionFinfo.baseName());
		}
		else{
			meshName = positionFinfo.baseName();
		}
		meshName.append("-SimUV");

		//Set the colors
		//Set the color boundaries as selected by the user
		float fromBlue, fromGreen, fromRed, toRed;
		if(ui->renderAmbiantRadioBtn->isChecked()){
			float ambiantIntensity = 0;
			if(ui->renderAmbiantDirectCheckBox->isChecked()){
				ambiantIntensity += flatSurfaceIntensity.direct;
			}
			if(ui->renderAmbiantDiffusedCheckBox->isChecked()){
				ambiantIntensity += flatSurfaceIntensity.diffused;
			}
			if(ui->renderAmbiantReflectedCheckBox->isChecked()){
				ambiantIntensity += flatSurfaceIntensity.reflected;
			}
			fromBlue = ambiantIntensity*ui->renderBlueFromSpinBox->value()/100.0f;
			fromGreen = ambiantIntensity*ui->renderGreenFromSpinBox->value()/100.0f;
			fromRed = ambiantIntensity*ui->renderRedFromSpinBox->value()/100.0f;
			toRed = ambiantIntensity*ui->renderRedToSpinBox->value()/100.0f;
		}
		else{
			fromBlue = ui->renderBlueFromSpinBox->value();
			fromGreen = ui->renderGreenFromSpinBox->value();
			fromRed = ui->renderRedFromSpinBox->value();
			toRed = ui->renderRedToSpinBox->value();
		}
		//Set the sources to be rendered
		int sources = 0;
		if(ui->renderDirectCheckBox->isEnabled() && ui->renderDirectCheckBox->isChecked()){
			sources += IntensitySources::DIRECT;
		}
		if(ui->renderDiffusedCheckBox->isEnabled() && ui->renderDiffusedCheckBox->isChecked()){
			sources += IntensitySources::DIFFUSED;
		}
		if(ui->renderReflectedCheckBox->isEnabled() && ui->renderReflectedCheckBox->isChecked()){
			sources += IntensitySources::REFLECTED;
		}
		if(isAggregate){
			//UVMesh::setColors(destMesh,uvModel->getUVMesh()->getZones(),flatSurfaceIntensity.diffused + flatSurfaceIntensity.direct + flatSurfaceIntensity.reflected);
			uvModel->getUVMesh()->setColorsFromZones(fromBlue, fromGreen, fromRed, toRed,sources);
		}
		else{
			//UVMesh::setColors(destMesh,uvModel->getUVMesh()->getEvaluatedIntensityList(),flatSurfaceIntensity.diffused + flatSurfaceIntensity.direct + flatSurfaceIntensity.reflected);
			uvModel->getUVMesh()->setColors(fromBlue, fromGreen, fromRed, toRed,sources);
		}

		// creating the new layer
		MeshModel *currentMesh  = uvModel->getUVMesh()->getModel();
		MeshModel *destMesh= OldApi::AddMeshModel(meshDoc, meshName.toStdString().c_str());
		vcg::tri::Append<CMeshO,CMeshO>::Mesh(destMesh->cm, currentMesh->cm, false, true); // the last true means "copy all vertices"

		// init new layer
		vcg::tri::UpdateBounding<CMeshO>::Box(destMesh->cm);						// updates bounding box
		CMeshO::FaceIterator fi;
		for(fi=destMesh->cm.face.begin();fi!=destMesh->cm.face.end();++fi)	// face normals
			OldApi::ComputeNormalizedNormal(*fi);
		vcg::tri::UpdateNormal<CMeshO>::PerVertex(destMesh->cm);				// vertex normals
		destMesh->cm.Tr = currentMesh->cm.Tr;								// copy transformation
		destMesh->updateDataMask(MeshModel::MM_VERTCOLOR);
	}
}

bool MainWindow::initUVModel(UVModel *uvModel, QString positionName){
	//General options
	uvModel->setRayPlaneTolerance(ui->rayPlaneSpinBox->value());

	//Bounding Boxes
	uvModel->setNumBoxes(ui->xBBSpinBox->value(),ui->yBBSpinBox->value(), ui->zBBSpinBox->value());
	uvModel->setDirectUseBoxes(ui->bbDirectCheckBox->isChecked());
	uvModel->setDiffuseUseBoxes(ui->bbDiffuseCheckBox->isChecked());
	uvModel->setReflectedUseBoxes(ui->bbReflectedCheckBox->isChecked());

	//Direct Source setup
	uvModel->setUseDirectSource(ui->useDirectSourceCheckBox->isChecked());
	
	//Diffuse Source setup
	uvModel->setUseDiffuseSource(ui->useDiffuseSourceCheckBox->isChecked());
	uvModel->setDiffuseLvlNb(ui->diffuseNumLevelSpinBox->value());
	uvModel->setDiffusePtNb(ui->diffuseNumPtsSpinBox->value());
	uvModel->setDiffuseRadiusFactor(ui->diffuseRadiusSpinBox->value());
	uvModel->setDiffuseAttenuationAngle(ui->attenuationAngleSpinBox->value());
	if(sources & IntensitySources::DIFFUSED && ui->diffuseMapCheckBox->isChecked()){
		if(!uvModel->setDiffuseMapPath(ui->diffuseMapLineEdit->text().replace("%position%",positionName).toStdString())){
			QMessageBox warningMessage(QMessageBox::Warning, "Diffused Map not found", "The diffused map for position " + positionName + " was not found or did not correspond to the position (different number of vertices). The calculation for this position might therefore be very long. Continue?",QMessageBox::Yes | QMessageBox::No, this);	
			if(warningMessage.exec() == QMessageBox::No){
				return false;
			}
			
		}
	}

	//Reflected Source setup
	uvModel->setUseReflectedSource(ui->useReflectedSourceCheckBox->isChecked());
	uvModel->setReflectedLvlNb(ui->reflectedNumLevelSpinBox->value());
	uvModel->setReflectedPtNb(ui->reflectedNumPtsSpinBox->value());
	uvModel->setReflectedRadiusFactor(ui->reflectedRadiusSpinBox->value());
	if(sources & IntensitySources::REFLECTED && ui->reflectedMapCheckBox->isChecked()){
		if(!uvModel->setReflectedMapPath(ui->reflectedMapLineEdit->text().replace("%position%",positionName).toStdString())){
			QMessageBox warningMessage(QMessageBox::Warning, "Reflected Map not found", "The reflected map for position " + positionName + " was not found or did not correspond to the position (different number of vertices). The calculation for this position might therefore be very long. Continue?",QMessageBox::Yes | QMessageBox::No, this);	
			if(warningMessage.exec() == QMessageBox::No){
				return false;
			}
		}
	}

	//Initiate the Model with the paramters set above
	uvModel->initModel();

	return true;

}

//ANATOMICAL ZONES
void MainWindow::loadAnatZones(){
	QString dir;
	if (lastDirectory == ""){
		dir=".";
	}else{
		dir = lastDirectory;
	}
	currentAnatFilename = QFileDialog::getOpenFileName(this,tr("Select Anatomical Zones Data File"),dir, "*.xml");
	if(!currentAnatFilename.isNull()){

		//Parse the zone file
		zones = IOZones::parseXMLFile(currentAnatFilename.toStdString().c_str());

		if(!zones){
			QMessageBox::information(0, "Failed to parse anatomical zones file.", "Error");
			return;
		}

		anatZonesChanged = false;

		//(Re-)Load the zones in the QTreeWidget
		reloadAnatZones();		


		//Setup GUI based on Excel file information
		QFileInfo finfo(currentAnatFilename);
		lastDirectory = finfo.absolutePath();
		ui->useAnatZonesLineEdit->setText(currentAnatFilename);
		checkProtectionsAgainstLibrary();

	}
}

QList<QTreeWidgetItem*> MainWindow::loadZonesInTree(vector<Zone*>* zones){
	vector<Zone*>::iterator zoneIter;
	QList<QTreeWidgetItem*> zoneList;

	for(zoneIter = zones->begin(); zoneIter != zones->end(); zoneIter++){
		QStringList zoneAttr(QString::fromStdString((*zoneIter)->getName()));
		vcg::Color4b *color = (*zoneIter)->getColor();
		if(color){
			zoneAttr << "" << QString::number((*color)[0]) << QString::number((*color)[1]) << QString::number((*color)[2]);
		}
		else{
			zoneAttr << "" << ""  << ""  << "";
		}
		QTreeWidgetItem *zoneItem = new QTreeWidgetItem(zoneAttr);
		//Set whether the zone is active
		if((*zoneIter)->isZoneActive()){
			zoneItem->setCheckState(AnatZonesTreeColumns::ZONE,Qt::Checked);
		}
		else{
			zoneItem->setCheckState(AnatZonesTreeColumns::ZONE,Qt::Unchecked);
		}
		//Set the zone to be editable
		zoneItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable ) ;

		//Set the color if any
		if(color){
			zoneItem->setBackgroundColor(AnatZonesTreeColumns::COLOR,QColor((*color)[0], (*color)[1], (*color)[2]));
		}
		if((*zoneIter)->getSubZones()){
			zoneItem->addChildren(loadZonesInTree((*zoneIter)->getSubZones()));
		}
		zoneList << zoneItem;
	}
	return zoneList;
}

void MainWindow::anatTreeItemChanged(QTreeWidgetItem *item, int column){
	
	//Set the anatzones changes
	anatZonesChanged = true;
	reloadAnatZonesChanged();
	
	//Changing zone name is fine in any case
	if(column == AnatZonesTreeColumns::ZONE){
		return;
	}

	//Check whether the item has children or not
	//If it has, signal to user that the modification is void
	if(item->childCount() > 0){
		QMessageBox::information(0, "Only anatomical zones without children can have a color.","Notice");
		item->setText(column,"");
		return;
	}
	
	//If the user attempts to change the color directly inform that this is no possible
	if(column == AnatZonesTreeColumns::COLOR){
		item->setText(column,"");
		return;
	}

	//Finally, if the color was changed, make sure it's within boundaries and change the color column
	bool ok;
	int newColor = item->text(column).toInt(&ok);
	if(!ok || newColor<0){
		QMessageBox::information(0, "RGB values can only be integer numbers between 0 and 255.","Notice");
		item->setText(column, "0");
	}
	else if(newColor > 255){
		QMessageBox::information(0, "RGB values can only be integer numbers between 0 and 255.","Notice");
		item->setText(column, "255");
	}

	item->setBackgroundColor(AnatZonesTreeColumns::COLOR,QColor(item->text(AnatZonesTreeColumns::RED).toInt(), item->text(AnatZonesTreeColumns::GREEN).toInt(), item->text(AnatZonesTreeColumns::BLUE).toInt()));

}

void MainWindow::addAnatZone(bool topLevel){
	
	//Create new Item
	QStringList zoneAttr(QString("New Zone"));
	zoneAttr << "" << ""  << ""  << "";
	QTreeWidgetItem *newItem = new QTreeWidgetItem(zoneAttr);
	newItem->setCheckState(AnatZonesTreeColumns::ZONE,Qt::Checked);
	newItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable ) ;

	if(topLevel){
		ui->anatZonesTreeWidget->addTopLevelItem(newItem);
	}
	else{ 
		QList<QTreeWidgetItem*> selectedItem = ui->anatZonesTreeWidget->selectedItems();
		if(selectedItem.count() == 1){
			selectedItem.at(0)->addChild(newItem);
		}
	}

	//Set the anatzones changes
	anatZonesChanged = true;
	reloadAnatZonesChanged();

}

void MainWindow::addTopLevelAnatZone(){
	addAnatZone(true);
}

void MainWindow::deleteAnatZone(){
	QList<QTreeWidgetItem*> selectedItem = ui->anatZonesTreeWidget->selectedItems();
	if(selectedItem.count() == 1){
		delete selectedItem.at(0);
		//Set the anatzones changes
		anatZonesChanged = true;
		reloadAnatZonesChanged();
	}
}

void MainWindow::reloadAnatZones(){
	//Delete the tree
	while( int nb = ui->anatZonesTreeWidget->topLevelItemCount () )
	{
		delete ui->anatZonesTreeWidget->takeTopLevelItem( nb - 1 );
	}

	//Reload the zones in the QTreeWidget
	if(zones){
		ui->anatZonesTreeWidget->addTopLevelItems(loadZonesInTree(zones));
		ui->anatZonesTreeWidget->expandAll();
	}

	//Reload the changed indicator
	reloadAnatZonesChanged();
}

void MainWindow::saveAnatZonesAs(){
	QString dir;
	if (lastDirectory == ""){
		dir=".";
	}else{
		dir = lastDirectory;
	}
	QString filename = QFileDialog::getSaveFileName(this,"Select File to Save Zones",dir, "*.xml");
	if(!filename.isNull()){
		currentAnatFilename = filename;
		QFileInfo finfo(currentAnatFilename);
		lastDirectory = finfo.absolutePath();	
		saveAnatZones();		
	}

}

void MainWindow::saveAnatZones(){
	if(currentAnatFilename.isEmpty()){
		saveAnatZonesAs();
	}
	else{
		IOZones::saveZonesFile(ui->anatZonesTreeWidget,currentAnatFilename.toStdString());	
		ui->useAnatZonesLineEdit->setText(currentAnatFilename);
		Helpers::deleteZones(&zones);
		zones = IOZones::parseXMLFile(currentAnatFilename.toStdString().c_str());
		anatZonesChanged = false;
		reloadAnatZones();
		checkProtectionsAgainstLibrary();
	}
}

/*void MainWindow::setAnatZonesExportName(){
	if(ui->exportAnatZonesLineEdit->text().isEmpty()){
		ui->exportAnatZonesLineEdit->setText(meshDoc->mm()->pathName()+"/"+anatZonesFilename);
	}
}*/

void MainWindow::reloadAnatZonesChanged(){
	if(anatZonesChanged){
		ui->advancedTabWidget->setTabText(AdvancedTabs::ANATZONES,"Anatomical Zones*");
		ui->anatomicalZonesGroupBox->setTitle("Anatomical Zones*");
	}
	else{
		ui->advancedTabWidget->setTabText(AdvancedTabs::ANATZONES,"Anatomical Zones");
		ui->anatomicalZonesGroupBox->setTitle("Anatomical Zones");
	}
}

//POI
void MainWindow::loadPOI(){
	QString dir;
	if (lastDirectory == ""){
		dir=".";
	}else{
		dir = lastDirectory;
	}
	QString filename = QFileDialog::getOpenFileName(this,tr("Select POI Data File"),dir, "*.pp");
	if(!filename.isNull()){

		//Parse the POI file
		poi = IOPoints::parseXMLFile(filename.toStdString().c_str());

		//(Re-)load the POIs
		reloadPOI();

		//Setup GUI based on Excel file information
		QFileInfo finfo(filename);
		lastDirectory = finfo.absolutePath();
		ui->usePOILineEdit->setText(filename);


	}
}

void MainWindow::reloadPOI(){
	while( int nb = ui->POITreeWidget->topLevelItemCount () )
	{
		delete ui->POITreeWidget->takeTopLevelItem( nb - 1 );
	}

	//Display the points
	if(poi){
		vector<POI>::iterator poiIter;
		for(poiIter = poi->begin(); poiIter != poi->end(); poiIter++){
			ui->POITreeWidget->addTopLevelItem(new QTreeWidgetItem(QStringList(QString::fromStdString(poiIter->getPointName()))));
		}
	}
}

/*void MainWindow::setPOIexportName(){
	if(ui->exportPOILineEdit->text().count() < 1){
		//ui->exportPOILineEdit->setText(baseName + "-POI_intensities.csv");
		ui->exportPOILineEdit->setText(meshDoc->mm()->pathName()+"/"+POIFilename);
	}
}*/

//PROTECTIONS

void MainWindow::loadProtectionsLib(){
QString dir;
	if (lastDirectory == ""){
		dir=".";
	}else{
		dir = lastDirectory;
	}
	currentProtectionsLibFilename = QFileDialog::getOpenFileName(this,tr("Select Protections Library File"),dir, "*.xml");
	if(!currentProtectionsLibFilename.isNull()){

		//Parse the Protections lib file
		Protections::loadProtectionsLib(currentProtectionsLibFilename.toStdString().c_str());

		//(Re)load the library
		reloadProtectionsLib();

	}
}

void MainWindow::reloadProtectionsLib(){
	//Empty the protections lib
	while( int nb = ui->protectionsLibClothesTreeWidget->topLevelItemCount () )
	{
		delete ui->protectionsLibClothesTreeWidget->takeTopLevelItem( nb - 1 );
	}

	while( int nb = ui->protectionsLibMaterialsTreeWidget->topLevelItemCount () )
	{
		delete ui->protectionsLibMaterialsTreeWidget->takeTopLevelItem( nb - 1 );
	}

	//Display the materials
	
	if(Protections::getMaterials()){
		map<string,int>::iterator materialIter;
		for(materialIter = Protections::getMaterials()->begin(); materialIter != Protections::getMaterials()->end(); materialIter++){
			QStringList materialAttr(QString::fromStdString(materialIter->first));
			materialAttr << QString::number(materialIter->second);
			QTreeWidgetItem* materialItem = new QTreeWidgetItem(materialAttr);
			//Set the material to be editable
			materialItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable ) ;
			ui->protectionsLibMaterialsTreeWidget->addTopLevelItem(materialItem);
		}
	}

	//Display the clothes	
	if(Protections::getClothes()){
		map<string,vector<string>*>::iterator clothesIter;
		vector<string>::iterator zonesIter;
		for(clothesIter = Protections::getClothes()->begin(); clothesIter != Protections::getClothes()->end(); clothesIter++){
			QTreeWidgetItem* clothing = new QTreeWidgetItem(QStringList(QString::fromStdString(clothesIter->first)));
			//Add the zones to the clothing
			for(zonesIter = clothesIter->second->begin(); zonesIter != clothesIter->second->end(); zonesIter++){
				clothing->addChild(new QTreeWidgetItem(QStringList(QString::fromStdString(*zonesIter))));
			}
			//Set the clothing name to be editable
			clothing->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable ) ;
			//Add the clothing to the clothes
			ui->protectionsLibClothesTreeWidget->addTopLevelItem(clothing);

		}
	}

	//Update the protections status
	checkProtectionsAgainstLibrary();

	reloadProtectionsChanged();

}

void MainWindow::clearProtectionsLib(){
	//Empty the protections lib UI
	while( int nb = ui->protectionsLibClothesTreeWidget->topLevelItemCount () )
	{
		delete ui->protectionsLibClothesTreeWidget->takeTopLevelItem( nb - 1 );
	}

	while( int nb = ui->protectionsLibMaterialsTreeWidget->topLevelItemCount () )
	{
		delete ui->protectionsLibMaterialsTreeWidget->takeTopLevelItem( nb - 1 );
	}

	//Empty the library in memory
	Protections::resetProtectionsLibrary();

	//Update the protections status
	checkProtectionsAgainstLibrary();

	currentProtectionsLibFilename = "";
	protectionsLibChanged = false;
	reloadProtectionsChanged();

}

void MainWindow::protectionsLibMaterialsItemChanged(QTreeWidgetItem *item,int column){
	//If the material name is changed, make sure it's not simply an integer since
	//it would be confusing with the IPs
	bool ok;
	int changedText = item->text(column).toInt(&ok);
	if(column == 0 && ok){
		 QMessageBox::information(0,"Material names cannot be simple Integer numbers.", "Notice");
		 item->setText(column, "Invalid Name");
	}
	//If the IP is changed, make sure it is an Integer greater than 0
	else if(column == 1 && (!ok || changedText<1)){
		QMessageBox::information(0,"IP values must be Integer greater than 0.","Notice");
		item->setText(column, "1");
	}
	
	protectionsLibChanged = true;
	reloadProtectionsChanged();

}

void MainWindow::addProtectionsLibMaterial(){
	QStringList materialAttr(QString("New Material"));
	materialAttr << "1";
	QTreeWidgetItem *newItem = new QTreeWidgetItem(materialAttr);
	newItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable ) ;
	ui->protectionsLibMaterialsTreeWidget->addTopLevelItem(newItem);
	protectionsLibChanged = true;
	reloadProtectionsChanged();
}

void MainWindow::deleteProtectionsLibMaterial(){
	QList<QTreeWidgetItem*> selectedItem = ui->protectionsLibMaterialsTreeWidget->selectedItems();
	if(selectedItem.count() == 1){
		delete selectedItem.at(0);
		protectionsLibChanged = true;
		reloadProtectionsChanged();
	}
}

void MainWindow::addProtectionsLibClothing(){
	QTreeWidgetItem *newItem = new QTreeWidgetItem(QStringList(QString("New Clothing")));
	newItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable ) ;
	ui->protectionsLibClothesTreeWidget->addTopLevelItem(newItem);
	protectionsLibChanged = true;
	reloadProtectionsChanged();
}

void MainWindow::protectionsLibClothingItemChanged(QTreeWidgetItem *item,int column){
	protectionsLibChanged = true;
	reloadProtectionsChanged();
}

void MainWindow::addProtectionsLibClothingZone(){
	
	//Make sure that there are anatomical zones
	if(ui->anatZonesTreeWidget->topLevelItemCount() < 1){
		QMessageBox::information(0,"You first need to load or create anatomical zones.","Notice");
		return;
	}

	//Select the appropriate Clothing node
	QList<QTreeWidgetItem*> selectedItem = ui->protectionsLibClothesTreeWidget->selectedItems();
	if(selectedItem.count() == 1){
		QTreeWidgetItem *parentItem = selectedItem.at(0);
		while(parentItem->parent()){
			parentItem = parentItem->parent();
		}

		//Create the Dialog
		SimUVAddClothingZonesDialog *dialog = new SimUVAddClothingZonesDialog(this);
		ui->setupUi(dialog);

		//Fill the dialog tree from the current anatomical zones
		for(int i=0;i<ui->anatZonesTreeWidget->topLevelItemCount();i++){
//			ui->zonesTreeWidget->addTopLevelItem(duplicateQTreeWidgetItem(ui->anatZonesTreeWidget->topLevelItem(i))); MODIFY
		}

		if(dialog->exec() == QDialog::Accepted){
//			selectedItem = ui->zonesTreeWidget->selectedItems();MODIFY
			QTreeWidgetItem *currentItem;
			bool parentSelected;
			for(int i=0; i<selectedItem.count();i++){
				//TODO: Eventually check that the new items or their parents are not already present in the clothing
				//Check that no parent of the selected item is selected
				currentItem = selectedItem.at(i);
				parentSelected = false;
				while(currentItem->parent()){
					currentItem = currentItem->parent();
					if(selectedItem.contains(currentItem)){
						parentSelected = true;
					}
				}
				if(!parentSelected){
					parentItem->addChild(duplicateQTreeWidgetItem(currentItem,1,false));
				}
			}
		}
		protectionsLibChanged = true;
		reloadProtectionsChanged();
	}
}

void MainWindow::deleteProtectionsLibClothing(){
	QList<QTreeWidgetItem*> selectedItem = ui->protectionsLibClothesTreeWidget->selectedItems();
	if(selectedItem.count() == 1){
		delete selectedItem.at(0);
		protectionsLibChanged = true;
		reloadProtectionsChanged();
		//Update the protections status
		checkProtectionsAgainstLibrary();
	}	
}

void MainWindow::saveProtectionsLibAs(){
	QString dir;
	if (lastDirectory == ""){
		dir=".";
	}else{
		dir = lastDirectory;
	}
	QString filename = QFileDialog::getSaveFileName(this,"Select File to Save Protections Library",dir, "*.xml");
	if(!filename.isNull()){
		currentProtectionsLibFilename = filename;
		QFileInfo finfo(currentProtectionsLibFilename);
		lastDirectory = finfo.absolutePath();
		saveProtectionsLib();
	}
}

void MainWindow::saveProtectionsLib(){
	if(currentProtectionsLibFilename.isEmpty()){
		saveProtectionsLibAs();
	}
	else{
		Protections::saveProtectionsLibraryAsXML(ui->protectionsLibClothesTreeWidget, ui->protectionsLibMaterialsTreeWidget, currentProtectionsLibFilename.toStdString().c_str());		
		Protections::loadProtectionsLib(currentProtectionsLibFilename.toStdString().c_str(),true);
		protectionsLibChanged = false;
		reloadProtectionsLib();		
	}
}

void MainWindow::loadProtections(){
	QString dir;
	if (lastDirectory == ""){
		dir=".";
	}else{
		dir = lastDirectory;
	}
	currentProtectionsFilename = QFileDialog::getOpenFileName(this,tr("Select Protections Data File"),dir, "*.xml");
	if(!currentProtectionsFilename.isNull()){

		//Parse the protections file
		Protections::loadProtections(currentProtectionsFilename.toStdString().c_str());

		//(Re-)load the protections
		reloadProtections();

		//Setup GUI based on Excel file information
		QFileInfo finfo(currentProtectionsFilename);
		lastDirectory = finfo.absolutePath();
		ui->useProtectionLinesEdit->setText(currentProtectionsFilename);
	}
}

void MainWindow::reloadProtections(){
	//Empty the protections lib UI
	while( int nb = ui->protectionsTreeWidget->topLevelItemCount () )
	{
		delete ui->protectionsTreeWidget->takeTopLevelItem( nb - 1 );
	}

	//Load the protections into the tree
	vector<Protection>* protections = Protections::getProtections();

	if(protections){

		for(vector<Protection>::iterator proIter = protections->begin(); proIter != protections->end(); proIter++){
			QStringList protectionAttr(QString::fromStdString(proIter->name));
			if(proIter->material.empty()){
				protectionAttr << "";			
			}
			else{
				protectionAttr << QString::fromStdString(proIter->material);
			}
			protectionAttr << QString::number(proIter->IP);
			if(proIter->zoneType == Protections::CLOTHINGTYPE){
				protectionAttr << "Clothing";
			}
			else{
				protectionAttr << "Zone";
			}

			QTreeWidgetItem *protectionItem = new QTreeWidgetItem(protectionAttr);
			if(proIter->active){
				protectionItem->setCheckState(ProtectionsTree::PROTECTION,Qt::Checked);
			}
			else{
				protectionItem->setCheckState(ProtectionsTree::PROTECTION,Qt::Unchecked);
			}
			ui->protectionsTreeWidget->addTopLevelItem(protectionItem);

		}

		//Update the protections status
		checkProtectionsAgainstLibrary();

		//Update the changed status
		reloadProtectionsChanged();
	}
}

void MainWindow::addProtection(){
	//Make sure that there are anatomical zones or clothes available
	if(ui->anatZonesTreeWidget->topLevelItemCount() < 1 && ui->protectionsLibClothesTreeWidget->topLevelItemCount() < 1){
		QMessageBox::information(0, "You first need to load or create anatomical zones or clothes.", "Notice");
		return;
	}


	//Create the Dialog
	SimUVAddProtectionsDialog *dialog = new SimUVAddProtectionsDialog(this);
	ui->setupUi(dialog);

	//Fill the dialog tree from the current anatomical zones and clothes
	//Load the clothes
	QTreeWidgetItem *clothesNode = new QTreeWidgetItem(QStringList("Clothes"));
	for(int i=0;i<ui->protectionsLibClothesTreeWidget->topLevelItemCount();i++){
		clothesNode->addChild(duplicateQTreeWidgetItem(ui->protectionsLibClothesTreeWidget->topLevelItem(i),1,false));		
	}
//	dialog->ui->areaTreeWidget->addTopLevelItem(clothesNode); MODIFY

	//Load the zones
	QTreeWidgetItem *zonesNode = new QTreeWidgetItem(QStringList("Zones"));
	for(int i=0;i<ui->anatZonesTreeWidget->topLevelItemCount();i++){
		zonesNode->addChild(duplicateQTreeWidgetItem(ui->anatZonesTreeWidget->topLevelItem(i)));	
	}
//	dialog->ui->areaTreeWidget->addTopLevelItem(zonesNode); MODIFY

	//Fill the material combobox
	if(ui->protectionsLibMaterialsTreeWidget->topLevelItemCount() < 1){
//		dialog->ui->materialComboBox->setEnabled(false); MODIFY
//		dialog->ui->materialRadioButton->setEnabled(false); MODIFY
	}
	else{
//		dialog->ui->materialRadioButton->setEnabled(true); MODIFY
		for(int i=0;i<ui->protectionsLibMaterialsTreeWidget->topLevelItemCount();i++){
//			dialog->ui->materialComboBox->addItem(ui->protectionsLibMaterialsTreeWidget->topLevelItem(i)->text(0)); MODIFY
		}
	}

	if(dialog->exec() == QDialog::Accepted){

		//Get and check Material or IP
		QString material;
		int IP;
/*		if(dialog->ui->materialRadioButton->isChecked()){
//			material = dialog->ui->materialComboBox->currentText(); MODY
			if(Protections::getMaterials() && (Protections::getMaterials()->find(material.toStdString()) != Protections::getMaterials()->end())){
				IP = (*Protections::getMaterials())[material.toStdString()];
			}
			else{
				IP = 1;
			}
		}
		else{
			material = "";
			IP = dialog->ui->IPSpinBox->value();
			
		}*/

//		QList<QTreeWidgetItem*> selectedItems = dialog->ui->areaTreeWidget->selectedItems();MODIFY
		QTreeWidgetItem *currentItem;
		bool parentSelected;
/*		for(int i=0; i<selectedItems.count();i++){
			currentItem = selectedItems.at(i);
			//If the item is the root of either Zones or Clothes, discard
			if(currentItem->parent()){
				
				//Set common attributes
				QStringList protectionAttr(currentItem->text(0));
				protectionAttr << material;
				protectionAttr << QString::number(IP);

				//Take care of Clothes
				if(currentItem->parent()->text(0) == "Clothes"){					
					protectionAttr << "Clothing";
					QTreeWidgetItem *protectionItem = new QTreeWidgetItem(protectionAttr);
					protectionItem->setCheckState(0,Qt::Checked);
					ui->protectionsTreeWidget->addTopLevelItem(protectionItem);
				}
				//Take care of Zones
				else{
					
					//Check that the parent of the selected zone is not selected too
					parentSelected = false;
					while(currentItem->parent()){
						currentItem = currentItem->parent();
						if(selectedItems.contains(currentItem)){
							parentSelected = true;
						}
					}
					if(!parentSelected){						
						protectionAttr << "Zone";
						QTreeWidgetItem *protectionItem = new QTreeWidgetItem(protectionAttr);
						protectionItem->setCheckState(0,Qt::Checked);
						ui->protectionsTreeWidget->addTopLevelItem(protectionItem);
					}
				}
			}
		}*/
		protectionsChanged = true;
		reloadProtectionsChanged();
		
		//Update the protections status
		checkProtectionsAgainstLibrary();

	}
}

void MainWindow::protectionsItemChanged(QTreeWidgetItem *,int){
	protectionsChanged = true;
	reloadProtectionsChanged();
}

void MainWindow::deleteProtection(){
	QList<QTreeWidgetItem*> selectedItem = ui->protectionsTreeWidget->selectedItems();
	if(selectedItem.count() == 1){
		delete selectedItem.at(0);
		protectionsChanged = true;
		reloadProtectionsChanged();
	}
}

void MainWindow::saveProtectionsAs(){
	QString dir;
	if (lastDirectory == ""){
		dir=".";
	}else{
		dir = lastDirectory;
	}
	QString filename = QFileDialog::getSaveFileName(this,"Select File to Save Protections.",dir, "*.xml");
	if(!filename.isNull()){
		currentProtectionsFilename = filename;
		QFileInfo finfo(currentProtectionsFilename);
		lastDirectory = finfo.absolutePath();
		saveProtections();
		ui->useProtectionLinesEdit->setText(filename);
	}
}

void MainWindow::saveProtections(){
	if(currentProtectionsFilename.isEmpty()){
		saveProtectionsAs();
	}
	else{
		Protections::saveProtectionsAsXML(ui->protectionsTreeWidget,currentProtectionsFilename.toStdString());
		Protections::loadProtections(currentProtectionsFilename.toStdString().c_str(),true);
		protectionsChanged = false;
		reloadProtections();
	}
}

void MainWindow::reloadProtectionsChanged(){
	
	if(protectionsChanged){
		ui->protectionsGroupBox->setTitle("Protections*");
		ui->protectionsGroupBox_2->setTitle("Protections*");
	}
	else{
		ui->protectionsGroupBox->setTitle("Protections");
		ui->protectionsGroupBox_2->setTitle("Protections");
	}

	if(protectionsLibChanged){
		ui->protectionsLibraryGroupBox->setTitle("Protections Library*");
	}
	else{
		ui->protectionsLibraryGroupBox->setTitle("Protections Library");
	}

	
	if(protectionsChanged || protectionsLibChanged){
		ui->advancedTabWidget->setTabText(AdvancedTabs::PROTECTIONS,"Protections*");
	}
	else{
		ui->advancedTabWidget->setTabText(AdvancedTabs::PROTECTIONS,"Protections");
	}
}

//POSITIONS
void MainWindow::reloadPositionsTab(){

	//Get the names of the positions available
	QStringList positionsList("");
	for(int i=0; i<meshDoc->size();i++){
		positionsList << meshDoc->getMesh(i)->fullName();
	}
	positionsList.removeAt(0);

	//Refresh each of the rows
	QComboBox *positionsCombo;
	QString selectedItem;
	QDateTimeEdit *startDateTime;
	QDateTimeEdit *endDateTime;
	QDateTime currentDateTime;
	for(int i=0; i<ui->positionsTableWidget->rowCount(); i++){
		//Refresh the positionsList
		positionsCombo = (QComboBox*)(ui->positionsTableWidget->cellWidget(i,PositionsTableColumns::POSITION));
		selectedItem = positionsCombo->currentText();
		positionsCombo->clear();
		positionsCombo->addItems(positionsList);
		for(int j=0;j<positionsCombo->count();j++){
			if(positionsCombo->itemText(j) == selectedItem){
				positionsCombo->setCurrentIndex(j);
			}
		}
		//Refresh the date and time edits
		startDateTime = (QDateTimeEdit*)(ui->positionsTableWidget->cellWidget(i,PositionsTableColumns::STARTTIME));
		endDateTime = (QDateTimeEdit*)(ui->positionsTableWidget->cellWidget(i,PositionsTableColumns::ENDTIME));
		if(minDate && maxDate){
			//Start
			startDateTime->setEnabled(true);
			currentDateTime = startDateTime->dateTime();
			startDateTime->setMinimumDateTime(*minDate);
			startDateTime->setMaximumDateTime(*maxDate);
			if(currentDateTime >= *minDate && currentDateTime <= *maxDate){
				startDateTime->setDateTime(currentDateTime);
			}
			else{
				startDateTime->setDateTime(startDateTime->minimumDateTime());
			}

			//End		
			endDateTime->setEnabled(true);
			currentDateTime = endDateTime->dateTime();
			endDateTime->setMinimumDateTime(*minDate);
			endDateTime->setMaximumDateTime(*maxDate);
			if(currentDateTime >= *minDate && currentDateTime <= *maxDate){
				endDateTime->setDateTime(currentDateTime);
			}
			else{
				endDateTime->setDateTime(endDateTime->maximumDateTime());
			}
		}
		else{
			startDateTime->setDisabled(true);
			endDateTime->setDisabled(true);
		}
		
	}
}

void MainWindow::createPositionsRow(){
		
		//Create the new row
		ui->positionsTableWidget->setRowCount(ui->positionsTableWidget->rowCount()+1);
		//Get the row index
		int rowIndex = ui->positionsTableWidget->rowCount()-1;

		//Add the activation checkbox
		QCheckBox *positionActive = new QCheckBox();
		positionActive->setChecked(true);
		ui->positionsTableWidget->setCellWidget(rowIndex,PositionsTableColumns::ACTIVE,positionActive);

		//Add the positions combo
		MeshModel *currentMesh;
		//ui->positionsTableWidget->setRowCount(meshDoc->size());
		//ui->positionsTableWidget->verticalHeader()->hide();
		//Get the names of the positions available
		//QStringList positionsList("");
		QComboBox *positionsCombo = new QComboBox();
		for(int i=0; i<meshDoc->size();i++){
			//positionsList << meshDoc->getMesh(i)->fullName();
			positionsCombo->addItem(meshDoc->getMesh(i)->fullName(),i);
		}
		//positionsList.removeAt(0);
		
		//positionsCombo->addItems(positionsList);
		ui->positionsTableWidget->setCellWidget(rowIndex,PositionsTableColumns::POSITION,positionsCombo);

		//Add the start/end time QDateTimeEdits
		QDateTimeEdit *startDateTime = new QDateTimeEdit();
		startDateTime->setDisplayFormat("dd.MM.yyyy HH:mm");
		QDateTimeEdit *endDateTime = new QDateTimeEdit();
		endDateTime->setDisplayFormat("dd.MM.yyyy HH:mm");

		if(minDate && maxDate){
			startDateTime->setMinimumDateTime(*minDate);
			startDateTime->setMaximumDateTime(*maxDate);
			startDateTime->setDateTime(startDateTime->minimumDateTime());
			endDateTime->setMinimumDateTime(*minDate);
			endDateTime->setMaximumDateTime(*maxDate);
			endDateTime->setDateTime(endDateTime->maximumDateTime());
		}
		else{
			startDateTime->setDisabled(true);
			endDateTime->setDisabled(true);
		}
		ui->positionsTableWidget->setCellWidget(rowIndex,PositionsTableColumns::STARTTIME,startDateTime);
		ui->positionsTableWidget->setCellWidget(rowIndex,PositionsTableColumns::ENDTIME,endDateTime);

		//Add the orientation combobox
		QComboBox *orientationCombo = new QComboBox();
		orientationCombo->addItem("Fixed");
		orientationCombo->addItem("Average");
		orientationCombo->addItem("Sequence");
		orientationCombo->setCurrentIndex(0);
		ui->positionsTableWidget->setCellWidget(rowIndex,PositionsTableColumns::ORIENTATION, orientationCombo);

		//Add the Start Angle
		QDoubleSpinBox *startAngle = new QDoubleSpinBox();
		startAngle->setMinimum(-360.0);
		startAngle->setMaximum(360.0);
		startAngle->setDecimals(1);
		startAngle->setSingleStep(1.0);
		startAngle->setValue(180);
		ui->positionsTableWidget->setCellWidget(rowIndex,PositionsTableColumns::STARTANGLE,startAngle);

		//Add the End Angle
		QDoubleSpinBox *endAngle = new QDoubleSpinBox();
		endAngle->setMinimum(-360.0);
		endAngle->setMaximum(360.0);
		endAngle->setDecimals(1);
		endAngle->setSingleStep(1.0);
		endAngle->setValue(180);
		endAngle->setDisabled(true);
		ui->positionsTableWidget->setCellWidget(rowIndex,PositionsTableColumns::ENDANGLE,endAngle);

		//Add the Angular Step
		QDoubleSpinBox *angularStep = new QDoubleSpinBox();
		angularStep->setMinimum(-360.0);
		angularStep->setMaximum(360.0);
		angularStep->setDecimals(1);
		angularStep->setSingleStep(1.0);
		angularStep->setValue(1.0);
		angularStep->setDisabled(true);
		ui->positionsTableWidget->setCellWidget(rowIndex,PositionsTableColumns::ANGULARSTEP,angularStep);

		//Add the Time step
		QTimeEdit *timeStep = new QTimeEdit();
		timeStep->setDisplayFormat("HH:mm");
		timeStep->setDisabled(true);
		timeStep->setMinimumTime(QTime(0,0,1));
		timeStep->setTime(QTime(0,1));
		ui->positionsTableWidget->setCellWidget(rowIndex,PositionsTableColumns::TIMESTEP,timeStep);

		//Connect signals and slots
		connect(orientationCombo, SIGNAL(currentIndexChanged(int)),this, SLOT(positionOrientationTypeChanged(int)));
		connect(positionActive, SIGNAL(stateChanged(int)),this,SLOT(positionChecked(int)));

		//Increase the current number of positions
		numPositions+=1;

		//Refresh the GUI regarding the multiple positions options
		refreshMultiPosOptions();
		
}

void MainWindow::positionOrientationTypeChanged(int selectedOrientation){
	int currentRow = ui->positionsTableWidget->currentRow();
	if(selectedOrientation == 0){
		ui->positionsTableWidget->cellWidget(currentRow,PositionsTableColumns::ENDANGLE)->setEnabled(false);
		ui->positionsTableWidget->cellWidget(currentRow,PositionsTableColumns::ANGULARSTEP)->setEnabled(false);
		ui->positionsTableWidget->cellWidget(currentRow,PositionsTableColumns::TIMESTEP)->setEnabled(false);
	}	
	else if(selectedOrientation == 1){
		ui->positionsTableWidget->cellWidget(currentRow,PositionsTableColumns::ENDANGLE)->setEnabled(true);
		ui->positionsTableWidget->cellWidget(currentRow,PositionsTableColumns::ANGULARSTEP)->setEnabled(true);
		ui->positionsTableWidget->cellWidget(currentRow,PositionsTableColumns::TIMESTEP)->setEnabled(false);
	}
	else{
		ui->positionsTableWidget->cellWidget(currentRow,PositionsTableColumns::ENDANGLE)->setEnabled(false);
		ui->positionsTableWidget->cellWidget(currentRow,PositionsTableColumns::ANGULARSTEP)->setEnabled(true);
		ui->positionsTableWidget->cellWidget(currentRow,PositionsTableColumns::TIMESTEP)->setEnabled(true);
	}

}

void MainWindow::deletePosition(){
	if(((QCheckBox*)ui->positionsTableWidget->cellWidget(ui->positionsTableWidget->currentRow(),0))->isChecked()){
		//Decrease the current number of positions
		numPositions-=1;

		//Refresh the GUI regarding the multiple positions options
		refreshMultiPosOptions();
	}
	ui->positionsTableWidget->removeRow(ui->positionsTableWidget->currentRow());
}

void MainWindow::positionChecked(int state){
	if(state == Qt::Checked){
		numPositions += 1;
	}
	else{
		numPositions -= 1;
	}
	refreshMultiPosOptions();
}

void MainWindow::refreshMultiPosOptions(){
	
	if(numPositions > 1 && ui->useAnatZonesCheckBox->isChecked()){
		//Enable the multiple positions options	
		ui->multiplePositionsAvgRadioButton->setEnabled(true);
		ui->multiplePositionsSeqRadioButton->setEnabled(true);
		ui->targetPositionLabel->setEnabled(true);
		ui->targetPositionComboBox->setEnabled(true);

		//Refresh the target positions combobox
		ui->targetPositionComboBox->clear();
		QString itemName;
		for(int i = 0; i < ui->positionsTableWidget->rowCount(); ++i){
			if(((QCheckBox*)ui->positionsTableWidget->cellWidget(i, PositionsTableColumns::ACTIVE))->isChecked()){
				ui->targetPositionComboBox->addItem(QString::number(i+1), i);
			}
		}
	}
	else{
		//Disable the multiple positions options
		ui->multiplePositionsAvgRadioButton->setDisabled(true);
		ui->multiplePositionsSeqRadioButton->setDisabled(true);
		ui->targetPositionLabel->setDisabled(true);
		ui->targetPositionComboBox->setDisabled(true);
		ui->multiplePositionsSeparateRadioButton->setChecked(true);
	}

}

//Rendering
void MainWindow::blueFromValueChanged(int value){
	ui->renderGreenFromSpinBox->setMinimum(value);
}
void MainWindow::greenFromValueChanged(int value){
	ui->renderRedFromSpinBox->setMinimum(value);
}
void MainWindow::redFromValueChanged(int value){
	ui->renderRedToSpinBox->setMinimum(value);
}

//Helper Functions

void MainWindow::checkProtectionsAgainstLibrary(){
	//Loop over each protections to check whether the material and clothing is
	//present in one of the loaded libs
	map<string,int>* materials = Protections::getMaterials();
	map<string,vector<string>*>* clothes = Protections::getClothes();
	bool notFound;

	for(int i=0; i<ui->protectionsTreeWidget->topLevelItemCount(); i++){
		notFound = false;
		QTreeWidgetItem *protection = ui->protectionsTreeWidget->topLevelItem(i);
		//Check if the material is not found
		if(protection->text(ProtectionsTree::MATERIAL).isEmpty()){
			protection->setTextColor(ProtectionsTree::MATERIAL,QColor(0,0,0));
		}
		else if(!materials || materials->find(protection->text(1).toStdString()) == materials->end()){
			protection->setTextColor(ProtectionsTree::MATERIAL,QColor(255,0,0));
			notFound = true;
		}
		else{
			protection->setTextColor(ProtectionsTree::MATERIAL,QColor(0,0,0));
			protection->setText(ProtectionsTree::IP,QString::number((*materials)[protection->text(1).toStdString()]));
		}
		//If it's a clothing check in the clothing lib
		if(protection->text(ProtectionsTree::TYPE) == "Clothing" && (!clothes || clothes->find(protection->text(0).toStdString()) == clothes->end())){
			protection->setTextColor(ProtectionsTree::PROTECTION,QColor(255,0,0));
			notFound = true;
		}
		//If it's a zone, check in the zones
		else if(protection->text(ProtectionsTree::TYPE) == "Zone" && !isZoneLoaded(zones,protection->text(0).toStdString())){
			protection->setTextColor(ProtectionsTree::PROTECTION,QColor(255,0,0));
			notFound = true;
		}
		else{
			protection->setTextColor(ProtectionsTree::PROTECTION,QColor(0,0,0));
		}
		//If part of the information needded in the protections is not found in the library
		//loaded, signal it visually to the user
		if(notFound){
			protection->setDisabled(true);
		}
		else if(protection->isDisabled()){
			protection->setDisabled(false);
		}
	}

}

bool MainWindow::isZoneLoaded(vector<Zone*>* searchedZones, std::string zoneName){
	//Make sure zones are loaded
	if(!searchedZones){
		return false;
	}
	
	//Check recursively all the zones to make sure the zone is loaded
	vector<Zone*>::iterator zoneIter;
	for(zoneIter = searchedZones->begin(); zoneIter != searchedZones->end(); zoneIter++){
		//Check if this is the zone
		if((*zoneIter)->getName() == zoneName){
			return true;
		}
		//Check if the zone is part of the subzones
		if(isZoneLoaded((*zoneIter)->getSubZones(),zoneName)){
			return true;
		}
	}
	return false;

}

QTreeWidgetItem* MainWindow::duplicateQTreeWidgetItem(QTreeWidgetItem *item, int numcolumns, bool duplicateChildren){
	QStringList newItemAttr(item->text(0));
	for(int i=1; i<numcolumns;i++){
		newItemAttr << item->text(i);
	}
	QTreeWidgetItem *newItem = new QTreeWidgetItem(newItemAttr);
	if(duplicateChildren){
		for(int i=0; i<item->childCount();i++){
			newItem->addChild(duplicateQTreeWidgetItem(item->child(i)));
		}
	}
	return newItem;
}

void MainWindow::exportDirBrowse(QString caption, QLineEdit* lineEdit){
	QString dir;
	QFileInfo finfo(lineEdit->text());
	if(finfo.dir().exists()){
		dir=finfo.dir().absolutePath();
	}
	else if (lastDirectory == ""){
		dir=".";
	}else{
		dir = lastDirectory;
	}
	//QString filename = QFileDialog::getSaveFileName(this,caption,dir, "*.csv");
	QString dirname = QFileDialog::getExistingDirectory(this,caption,dir);
	if(!dirname.isNull()){
		
		lastDirectory = dirname;		
		lineEdit->setText(dirname+"/"+finfo.fileName());
	}
}

void MainWindow::updateExportPaths()
{
	const QString path = exportPath();
	ui->exportVertCSVLineEdit->setText(path + "/%position%-vertex_intensities.csv");
	ui->exportFlatCSVLineEdit->setText(path + "/%position%-control_intensities.csv");
	ui->exportAnatZonesLineEdit->setText(path + "/%position%-anatZones_intensities.csv");
	ui->exportPOILineEdit->setText(path + "/%position%-POI_intensities.csv");
	ui->exportProtectionsLineEdit->setText(path + "/%intensities%-protections.csv");
}

QString MainWindow::exportPath() const
{
	QString path = meshDoc->mm()->pathName();
	if(ui->checkBoxSubdirectory->isChecked())
	{
		QString subdirectory = ui->lineEditSubdirectory->text();
		if(!subdirectory.isEmpty())
			path += "/" + subdirectory;
	}
	return path;
}
