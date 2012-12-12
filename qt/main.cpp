#include "simulationcraft.hpp"
#include "simulationcraftqt.hpp"
#include <QLocale>
#ifdef QT_VERSION_5
#include <QtGui/QGuiApplication>
#else
#include <QtGui/QApplication>
#endif
#include <locale>
#ifndef SIMC_NO_AUTOUPDATE
#include "sc_autoupdate.h"
#endif /* SIMC_NO_AUTOUPDATE */

int main( int argc, char *argv[] )
{
  QLocale::setDefault( QLocale( "C" ) );
  std::locale::global( std::locale( "C" ) );
  setlocale( LC_ALL, "C" );



  dbc_t::init();

  QApplication a( argc, argv );
#ifndef SIMC_NO_AUTOUPDATE
  AutoUpdater* updater = 0;

  // Localization
  QTranslator qtTranslator;
  qtTranslator.load( "qt_" + QLocale::system().name(),
                     QLibraryInfo::location( QLibraryInfo::TranslationsPath ) );
  a.installTranslator( &qtTranslator );

  QString path_to_locale = QString( "locale" );

  QTranslator myappTranslator;
  myappTranslator.load( QString( "sc_" ) + QLocale::system().name(), path_to_locale );
  a.installTranslator( &myappTranslator );

#ifdef Q_WS_MAC

  CocoaInitializer cocoaInitializer;
  updater = new SparkleAutoUpdater( "http://simc.rungie.com/simcqt/update.xml" );

#endif

  if ( updater )
  {
    updater->checkForUpdates();
  }
#endif /* SIMC_NO_AUTOUPDATE */

#ifdef Q_WS_MAC
  QDir::home().mkpath( "Library/Application Support/simcqt" );
  QDir::setCurrent( QDir::home().absoluteFilePath( "Library/Application Support/simcqt" ) );
#endif

  // Setup search paths for resources
  {
    QString appDirPath = a.applicationDirPath();
    QDir::addSearchPath( "icon", ":/icons/" );
    QDir::addSearchPath( "icon", QString( "%1/../Resources/" ).arg( appDirPath ) );
    QDir::addSearchPath( "icon", QString( "%1/qt/icons/" ).arg( appDirPath ) );
    QDir::addSearchPath( "icon", "./qt/icons/" );
  }

  SC_MainWindow w;

  if ( w.historyWidth  != 0 &&
       w.historyHeight != 0 )
  {
    w.resize( w.historyWidth, w.historyHeight );
  }

  if ( w.historyMaximized )
  {
    w.showMaximized();
  }
  else
  {
    w.showNormal();
  }

  w.setWindowTitle( "SimulationCraft " + QString( SC_MAJOR_VERSION ) + "-" + QString( SC_MINOR_VERSION ) );

  w.cmdLine->setFocus();

  if ( argc > 1 )
  {
    for ( int i=1; i < argc; i++ )
    {
      QFile file( argv[ i ] );

      if ( file.open( QIODevice::ReadOnly ) )
      {
        w.simulateText->appendPlainText( file.readAll() );
        file.close();
      }
    }
    w.mainTab -> setCurrentTab( TAB_SIMULATE );
  }

  return a.exec();
}
