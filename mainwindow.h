#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>

#include <common/meshmodel.h>
#include <common/interfaces.h>
#include <QProgressDialog>
#include <wrap/gl/trimesh.h>
#include "IOSpreadSheet.h"
#include "IOZones.h"
#include "IOPoints.h"
#include "protections.h"
#include "UVModel.h"
#include <vcg/complex/append.h>
#include <map>
#include <vector>

namespace Ui {
class MainWindow;
class SimUVAddClothingZones;
class SimUVAddProtections;
}

class SimUVAddClothingZonesDialog: 
	public QDialog
{
public:

	Ui::SimUVAddClothingZones *ui;

	SimUVAddClothingZonesDialog(QWidget *parent = 0):QDialog(parent){
	}

};

class SimUVAddProtectionsDialog: 
	public QDialog
{
public:

	Ui::SimUVAddProtections *ui;

	SimUVAddProtectionsDialog(QWidget *parent = 0):QDialog(parent){
	}

};

struct AdvancedTabs{
	enum type {POSITIONS,MODEL,ANATZONES,POIS,PROTECTIONS};
};

struct AnatZonesTreeColumns{
	enum type{ZONE,COLOR,RED,GREEN,BLUE};
};

struct ProtectionsTree{
	enum type {PROTECTION,MATERIAL,IP,TYPE};
};
struct PositionsTableColumns{
	enum type {ACTIVE,POSITION,STARTTIME,ENDTIME,ORIENTATION,STARTANGLE,ENDANGLE,ANGULARSTEP,TIMESTEP};
};
struct PositionsOrientations{
	enum type {FIXED,AVERAGE,SEQUENCE};
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:

    Ui::MainWindow *ui;

	MeshDocument *meshDoc;
    QString lastDirectory;
    IOSpreadSheet *spreadsheet;
    QDateTime *maxDate;
	QDateTime *minDate;

	vector<Zone*>* zones;
	vector<POI>* poi;

	bool weatherDataLoaded;

	int sources;

	int numPositions;

	bool anatZonesChanged;
	bool protectionsLibChanged;
	bool protectionsChanged;

	QString currentAnatFilename;
	QString currentProtectionsLibFilename;
	QString currentProtectionsFilename;

private slots:
	void loadWeatherDataFile();
	void exportVertCSVFile();
	void exportControlCSVFile();
	void exportDiffuseMapFile();
	void exportReflectedMapFile();
	void filePositionLineEditChanged();
	void fileIntensitiesLineEditChanged();
	void runSimulation();

	//Anatomical Zones
	void exportZonesCSVFile();
	void loadAnatZones();
	void anatTreeItemChanged(QTreeWidgetItem *item, int column);
	void addAnatZone(bool topLevel = false);
	void addTopLevelAnatZone();
	void deleteAnatZone();
	void reloadAnatZones();
	void saveAnatZones();
	void saveAnatZonesAs();

	//POIs
	void exportPOICSVFile();
	void loadPOI();
	void reloadPOI();

	//Protections
	void loadProtectionsLib();
	void reloadProtectionsLib();
	void clearProtectionsLib();
	void protectionsLibMaterialsItemChanged(QTreeWidgetItem *item,int column);
	void addProtectionsLibMaterial();
	void deleteProtectionsLibMaterial();
	void protectionsLibClothingItemChanged(QTreeWidgetItem *item,int column);
	void addProtectionsLibClothing();
	void addProtectionsLibClothingZone();
	void deleteProtectionsLibClothing();
	void saveProtectionsLib();
	void saveProtectionsLibAs();
	void loadProtections();
	void reloadProtections();
	void addProtection();
	void deleteProtection();
	void saveProtections();
	void saveProtectionsAs();
	void exportProtectionsFile();
	void protectionsItemChanged(QTreeWidgetItem *,int);

	//Positions
	void reloadPositionsTab();
	void createPositionsRow();
	void deletePosition();
	void positionOrientationTypeChanged(int selectedOrientation);
	void positionChecked(int state);
	void refreshMultiPosOptions();

	//Rendering
	void blueFromValueChanged(int value);
	void greenFromValueChanged(int value);
	void redFromValueChanged(int value);

	//Export paths
	void updateExportPaths();

signals:
	void closing();

protected:
	virtual void closeEvent(QCloseEvent *event);
	virtual bool eventFilter(QObject *object, QEvent *event);


private:
	struct Position
	{
		bool checked;
		QDateTime qStartDate;
		QDateTime qEndDate;
		Date startDate;
		Date endDate;
		int meshIndex;
		int orientation;
		float startAngle;
		float endAngle;
		float angleStep;
		QTime timeStep;

	};

	//Helper functions
	void exportSimulationResults(UVModel *uvModel, QString positionName, bool exportProtectionsSeparately, bool isAggregate,
								 IOZones::OutputStreamMap &outputStreams, bool exportTotals);
	void simulatePosition(UVModel *uvModel, const Date &startDate, const Date &endDate, const Position &position);
	bool initUVModel(UVModel *uvModel, QString positionName);
	void reloadAnatZonesChanged();
	void reloadProtectionsChanged();
	void checkProtectionsAgainstLibrary();
	bool isZoneLoaded(vector<Zone*>* searchedZones, std::string zoneName);
	void exportDirBrowse(QString caption, QLineEdit* lineEdit);
	QList<QTreeWidgetItem*> loadZonesInTree(vector<Zone*>*);
	QTreeWidgetItem* duplicateQTreeWidgetItem(QTreeWidgetItem *item, int numcolumns = 1, bool duplicateChildren = true);

	void loadSettings();
	void saveSettings();
	QString exportPath() const;
};

#endif // MAINWINDOW_H
