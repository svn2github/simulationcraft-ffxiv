// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#ifndef SIMULATIONCRAFTQT_H
#define SIMULATIONCRAFTQT_H

#include "simulationcraft.hpp"
#include <QtGui/QtGui>
#include <QtWebKit/QtWebKit>
#include <QtCore/QTranslator>
#include <QtNetwork/QtNetwork>

#ifdef QT_VERSION_5
#include <QtWidgets/QtWidgets>
#include <QtWebKitWidgets/QtWebKitWidgets>
#endif

class SC_MainWindow;
#ifdef SC_PAPERDOLL
#include "simcpaperdoll.hpp"
class Paperdoll;
class PaperdollProfile;
#endif

enum main_tabs_e
{
  TAB_WELCOME = 0,
  TAB_OPTIONS,
  TAB_IMPORT,
  TAB_SIMULATE,
  TAB_OVERRIDES,
  TAB_HELP,
  TAB_LOG,
  TAB_RESULTS,
  TAB_SITE
#ifdef SC_PAPERDOLL
  , TAB_PAPERDOLL
#endif
};

enum import_tabs_e
{
  TAB_BATTLE_NET = 0,
  TAB_CHAR_DEV,
  TAB_RAWR,
  TAB_BIS,
  TAB_HISTORY,
  TAB_CUSTOM
};

class SC_WebView;
class SC_CommandLine;
class SimulateThread;
#ifdef SC_PAPERDOLL
class PaperdollThread;
#endif
class ImportThread;

// ============================================================================
// SC_StringHistory
// ============================================================================

class SC_StringHistory : public QStringList
{
public:
  int current_index;
  SC_StringHistory() : current_index( -1 ) {}
  QString current()
  {
    if ( current_index < 0 ) return "";
    if ( current_index >= size() ) return "";
    return at( current_index );
  }
  QString backwards()
  {
    if ( current_index < size() ) current_index++;
    return current();
  }
  QString forwards()
  {
    if ( current_index >= 0 ) current_index--;
    return current();
  }
  QString next( int k )
  {
    return ( k == Qt::Key_Up ) ? backwards() : forwards();
  }
  void add( QString s )
  {
    current_index = -1;
    if ( size() > 0 ) if ( at( 0 ) == s ) return;
    prepend( s );
  }
  QString start()
  {
    current_index = 0;
    return current();
  }
};

// ============================================================================
// PersistentCookieJar
// ============================================================================

class PersistentCookieJar : public QNetworkCookieJar
{
public:
  QString fileName;
  PersistentCookieJar( const QString& fn ) : fileName( fn ) {}
  virtual ~PersistentCookieJar() {}
  void load();
  void save();
};

// ============================================================================
// SC_PlainTextEdit
// ============================================================================

class SC_TextEdit : public QTextEdit
{
  Q_OBJECT
private:
  QTextCharFormat textformat_default;
  QTextCharFormat textformat_error;
public:
  bool edited_by_user;

  SC_TextEdit( QWidget* parent = 0, bool accept_drops = true ) :
    QTextEdit( parent ),
    edited_by_user( false )
  {
    textformat_error.setFontPointSize( 20 );

    setAcceptDrops( accept_drops );
    setLineWrapMode( QTextEdit::NoWrap );

    connect( this, SIGNAL( textChanged() ), this, SLOT( text_edited() ) );
  }

  void setformat_error()
  { //setCurrentCharFormat( textformat_error );
  }

  void resetformat()
  { //setCurrentCharFormat( textformat_default );
  }

  /*
  protected:
  virtual void dragEnterEvent( QDragEnterEvent* e )
  {
    e -> acceptProposedAction();
  }
  virtual void dropEvent( QDropEvent* e )
  {
    appendPlainText( e -> mimeData()-> text() );
    e -> acceptProposedAction();
  }
  */

private slots:

  void text_edited()
  { edited_by_user = true; }
};

// ============================================================================
// SC_ReforgeButtonGroup
// ============================================================================

class SC_ReforgeButtonGroup : public QButtonGroup
{
  Q_OBJECT
public:
  SC_ReforgeButtonGroup( QObject* parent = 0 );

private:
  int selected;

public slots:
  void setSelected( int state );
};


// ============================================================================
// SC_enumeratedTabWidget template
// ============================================================================

template <typename E>
class SC_enumeratedTab : public QTabWidget
{
public:
  SC_enumeratedTab( QWidget* parent = 0 ) :
    QTabWidget( parent )
  {

  }

  E currentTab()
  { return static_cast<E>( currentIndex() ); }

  void setCurrentTab( E t )
  { return setCurrentIndex( static_cast<int>( t ) ); }
};

// ============================================================================
// SC_MainTabWidget
// ============================================================================

class SC_MainTab : public SC_enumeratedTab<main_tabs_e>
{
  Q_OBJECT
public:
  SC_MainTab( QWidget* parent = 0 ) :
    SC_enumeratedTab<main_tabs_e>( parent )
  {

  }
};

// ============================================================================
// SC_WelcomeTabWidget
// ============================================================================

class SC_WelcomeTabWidget : public QWebView
{
  Q_OBJECT
public:
  SC_WelcomeTabWidget( SC_MainWindow* parent = nullptr );
};

// ============================================================================
// SC_ResultTabWidget
// ============================================================================

enum result_tabs_e
{
  TAB_HTML = 0,
  TAB_TEXT,
  TAB_XML,
  TAB_PLOTDATA,
  TAB_CSV
};

class SC_ResultTab : public QTabWidget
{
  Q_OBJECT
  SC_MainWindow* mainWindow;
public:
  SC_ResultTab( SC_MainWindow* );
public slots:
  void TabCloseRequest( int index )
  {
    assert( index < count() );
    if ( count() > 0 )
    {
      int confirm = QMessageBox::question( this, tr( "Close Results Tab" ), tr( "Do you really want to close these results?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes );
      if ( confirm == QMessageBox::Yes )
      {
        setCurrentIndex( index - 1 );
        removeTab( index );
      }
    }
  }
};

class SC_SingleResultTab : public SC_enumeratedTab<result_tabs_e>
{
  Q_OBJECT
  SC_MainWindow* mainWindow;
public:
  SC_SingleResultTab( SC_MainWindow* mw, QWidget* parent = 0 ) :
    SC_enumeratedTab<result_tabs_e>( parent ),
    mainWindow( mw )
  {

    connect( this, SIGNAL( currentChanged( int ) ), this, SLOT( TabChanged( int ) ) );
  }

  void save_result();

public slots:
  void TabChanged( int index );
};

// ============================================================================
// SC_ImportTabWidget
// ============================================================================

class SC_ImportTab : public SC_enumeratedTab<import_tabs_e>
{
  Q_OBJECT
public:
  SC_ImportTab( QWidget* parent = 0 ) :
    SC_enumeratedTab<import_tabs_e>( parent )
  {
  }
};

// ==========================================================================
// Utilities
// ==========================================================================


const QString defaultSimulateText( "# Profile will be downloaded into a new tab.\n"
                                   "#\n"
                                   "# Clicking Simulate will create a simc_gui.simc profile for review.\n" );

// ============================================================================
// SC_SimulateTabWidget
// ============================================================================

class SC_SimulateTab : public QTabWidget
{
  Q_OBJECT
  QWidget* addTabWidget;
public:
  SC_SimulateTab( QWidget* parent = nullptr ) :
    QTabWidget( parent ),
    addTabWidget( new QWidget( this ) )
  {
    setTabsClosable( true );
   // setMovable( true ); # Would need to disallow moving the + tab, or to the right of it. That would require subclassing tabbar

    int i = addTab( addTabWidget, QIcon( ":/icon/addtab.png" ), "" );
    tabBar() -> setTabButton( i, QTabBar::LeftSide, nullptr );
    tabBar() -> setTabButton( i, QTabBar::RightSide, nullptr );
    connect( this, SIGNAL( currentChanged(int) ), this, SLOT( addNewTab(int) ) );
    connect( this, SIGNAL( tabCloseRequested( int ) ), this, SLOT( TabCloseRequest( int ) ) );

  }

  static void format_document( SC_TextEdit* /*s*/ )
  {
    // Causes visual disappearance of text. Deactivated for now.
    /*QTextDocument* d = s -> document();
    QTextBlock b = d -> begin();
    QRegExp comment_rx( "^\\s*\\#" );
    QTextCharFormat* comment_format = new QTextCharFormat();
    comment_format -> setForeground( QColor( "forestgreen" ) );
    while ( b.isValid() )
    {
      if ( comment_rx.indexIn( b.text() ) != -1 )
      {
        QTextCursor c(b);
        c.select( QTextCursor::BlockUnderCursor );
        c.setCharFormat( *comment_format );
      }
      b = b.next();
    }*/
  }

  int add_Text( const QString& text, const QString& tab_name )
  {
    SC_TextEdit* s = new SC_TextEdit( this );
    s -> setText( text );
    format_document( s );
    int indextoInsert = indexOf( addTabWidget );
    int i = insertTab( indextoInsert, s, tab_name );
    setCurrentIndex( i );
    return i;
  }

  SC_TextEdit* current_Text()
  {
    return static_cast<SC_TextEdit*>( currentWidget() );
  }

  void set_Text( const QString& text )
  {
    SC_TextEdit* current_s = static_cast<SC_TextEdit*>( currentWidget() );
    current_s -> setText( text );
    format_document( current_s );
  }

  void append_Text( const QString& text )
  {
    SC_TextEdit* current_s = static_cast<SC_TextEdit*>( currentWidget() );
    current_s -> append( text );
    format_document( current_s );
  }
public slots:
  void TabCloseRequest( int index )
  {
    if ( count() > 2 && index != (count() - 1) )
    {
      int confirm = QMessageBox::question( this, tr( "Close Simulate Tab" ), tr( "Do you really want to close this simulation profile?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes );
      if ( confirm == QMessageBox::Yes )
      {
        setCurrentIndex( index - 1 );
        removeTab( index );
      }
    }
  }
  void addNewTab(int index )
  {
    if ( index == indexOf( addTabWidget ) )
    {
      SC_TextEdit* s = new SC_TextEdit( this );
      s -> setText( defaultSimulateText );
      insertTab( index, s, "Simulate" );
      setCurrentIndex( index);
    }
  }
};

class SC_OptionsTab;

// ============================================================================
// SC_MainWindow
// ============================================================================

class SC_MainWindow : public QWidget
{
  Q_OBJECT
public:
  QWidget* customGearTab;
  QWidget* customTalentsTab;
  QWidget* customGlyphsTab;
  SC_MainTab* mainTab;
  SC_OptionsTab* optionsTab;
  SC_ImportTab* importTab;
  SC_SimulateTab* simulateTab;
  SC_ResultTab* resultsTab;
  QTabWidget* createCustomProfileDock;
#ifdef SC_PAPERDOLL
  QTabWidget* paperdollTab;
  Paperdoll* paperdoll;
  PaperdollProfile* paperdollProfile;
#endif


  SC_WebView* battleNetView;
  SC_WebView* charDevView;
  SC_WebView* siteView;
  SC_WebView* helpView;
  SC_WebView* visibleWebView;
  PersistentCookieJar* charDevCookies;
  QPushButton* rawrButton;
  QByteArray rawrDialogState;
  SC_TextEdit* rawrText;
  QListWidget* historyList;
  SC_TextEdit* overridesText;
  SC_TextEdit* logText;
  QPushButton* backButton;
  QPushButton* forwardButton;
  SC_CommandLine* cmdLine;
  QProgressBar* progressBar;
  QPushButton* mainButton;
  QGroupBox* cmdLineGroupBox;
  QGroupBox* createCustomCharData;


  QTimer* timer;
  ImportThread* importThread;
  SimulateThread* simulateThread;
#ifdef SC_PAPERDOLL
  PaperdollThread* paperdollThread;
#endif

  sim_t* sim;
  sim_t* paperdoll_sim;
  std::string simPhase;
  int simProgress;
  int simResults;
//  QStringList resultsHtml;

  QString AppDataDir;
  QString ResultsDestDir;
  QString TmpDir;

  QString cmdLineText;
  QString logFileText;
  QString resultsFileText;

  void    startImport( int tab, const QString& url );
  void    startSim();
  sim_t*  initSim();
  void    deleteSim( sim_t* sim, SC_TextEdit* append_error_message = 0 );

  void saveLog();
  void saveResults();

  void loadHistory();
  void saveHistory();

  void createCmdLine();
  void createWelcomeTab();
  void createOptionsTab();
  void createImportTab();
  void createRawrTab();
  void createBestInSlotTab();
  void createCustomTab();
  void createSimulateTab();
  void createOverridesTab();
  void createHelpTab();
  void createLogTab();
  void createResultsTab();
  void createSiteTab();
  void createToolTips();
#ifdef SC_PAPERDOLL
  void createPaperdoll();
#endif
  void updateVisibleWebView( SC_WebView* );

protected:
  virtual void closeEvent( QCloseEvent* );

private slots:
  void importFinished();
  void simulateFinished( sim_t* );
#ifdef SC_PAPERDOLL
  void paperdollFinished();
  player_t* init_paperdoll_sim( sim_t*& );
  void start_intermediate_paperdoll_sim();
  void start_paperdoll_sim();
#endif
  void updateSimProgress();
  void cmdLineReturnPressed();
  void cmdLineTextEdited( const QString& );
  void backButtonClicked( bool checked = false );
  void forwardButtonClicked( bool checked = false );
  void mainButtonClicked( bool checked = false );
  void mainTabChanged( int index );
  void importTabChanged( int index );
  void resultsTabChanged( int index );
  void resultsTabCloseRequest( int index );
  void rawrButtonClicked( bool checked = false );
  void historyDoubleClicked( QListWidgetItem* item );
  void bisDoubleClicked( QTreeWidgetItem* item, int col );
  void armoryRegionChanged( const QString& region );

public:
  SC_MainWindow( QWidget *parent = 0 );
};

// ============================================================================
// SC_CommandLine
// ============================================================================

class SC_CommandLine : public QLineEdit
{
private:
  SC_MainWindow* mainWindow;

  virtual void keyPressEvent( QKeyEvent* e );
public:
  SC_CommandLine( SC_MainWindow* mw ) : mainWindow( mw ) {}
};

// ============================================================================
// SC_WebPage
// ============================================================================

class SC_WebPage : public QWebPage
{
  Q_OBJECT
public:
  explicit SC_WebPage( QObject* parent = 0 ) :
    QWebPage( parent )
  {}

  QString userAgentForUrl( const QUrl& /* url */ ) const
  { return QString( "simulationcraft_gui" ); }
};

// ============================================================================
// SC_WebView
// ============================================================================

class SC_WebView : public QWebView
{
  Q_OBJECT
public:
  SC_MainWindow* mainWindow;
  int progress;
  QString html_str;

  SC_WebView( SC_MainWindow* mw, QWidget* parent = 0, const QString& h = QString() ) :
    QWebView( parent ),
    mainWindow( mw ), progress( 0 ), html_str( h )
  {
    connect( this, SIGNAL( loadProgress( int ) ),        this, SLOT( loadProgressSlot( int ) ) );
    connect( this, SIGNAL( loadFinished( bool ) ),       this, SLOT( loadFinishedSlot( bool ) ) );
    connect( this, SIGNAL( urlChanged( const QUrl& ) ),  this, SLOT( urlChangedSlot( const QUrl& ) ) );
    connect( this, SIGNAL( linkClicked( const QUrl& ) ), this, SLOT( linkClickedSlot( const QUrl& ) ) );

    SC_WebPage* page = new SC_WebPage( this );
    setPage( page );
    page -> setLinkDelegationPolicy( QWebPage::DelegateExternalLinks );

    // Add QT Major Version to avoid "mysterious" problems resulting in qBadAlloc. Qt4 and Qt5 webcache do not like each other
    QDir dir( mainWindow -> TmpDir + QDir::separator() + "simc_webcache_qt" + std::string( QT_VERSION_STR ).substr( 0, 3 ).c_str() );
    if ( ! dir.exists() ) dir.mkpath( "." );

    QFileInfo fi( dir.absolutePath() );

    if ( fi.isDir() && fi.isWritable() )
    {
      QNetworkDiskCache* diskCache = new QNetworkDiskCache( this );
      diskCache -> setCacheDirectory( dir.absolutePath() );
      QString test = diskCache -> cacheDirectory();
      page -> networkAccessManager()->setCache( diskCache );
    }
    else
    {
      qDebug() << "Can't write webcache! sucks";
    }


  }
  void store_html( const QString& s )
  { html_str = s; }

  virtual ~SC_WebView() {}

  void loadHtml()
  { setHtml( html_str ); }

private:
  void update_progress( int p )
  {
    progress = p;
    if ( mainWindow -> visibleWebView == this )
    {
      mainWindow -> progressBar -> setValue( progress );
    }
  }

private slots:
  void loadProgressSlot( int p )
  { update_progress( p ); }
  void loadFinishedSlot( bool /* ok */ )
  { update_progress( 100 ); }
  void urlChangedSlot( const QUrl& url )
  {
    if ( mainWindow->visibleWebView == this )
    {
      QString s = url.toString();
      if ( s == "about:blank" ) s = "results.html";
      mainWindow->cmdLine->setText( s );
    }
  }
  void linkClickedSlot( const QUrl& url )
  { load( url ); }
};

// ============================================================================
// SimulateThread
// ============================================================================

class SimulateThread : public QThread
{
  Q_OBJECT
  SC_MainWindow* mainWindow;
  sim_t* sim;

public:
  QString options;
  bool success;

  void start( sim_t* s, const QString& o ) { sim = s; options = o; success = false; QThread::start(); }
  virtual void run();
  SimulateThread( SC_MainWindow* mw ) : mainWindow( mw ), sim( 0 )
  {
    connect( this, SIGNAL( finished() ), this, SLOT( sim_finished() ) );
  }
private slots:
  void sim_finished()
  { emit simulationFinished( sim ); }

signals:
  void simulationFinished( sim_t* s );
};

class ImportThread : public QThread
{
  Q_OBJECT
  SC_MainWindow* mainWindow;
  sim_t* sim;

public:
  int tab;
  QString url;
  QString profile;
  QString item_db_sources;
  QString active_spec;
  QString m_role;
  player_t* player;

  void importBattleNet();
  void importCharDev();
  void importRawr();

  void start( sim_t* s, int t, const QString& u, const QString& sources, const QString& spec, const QString& role )
  { sim = s; tab = t; url = u; profile = ""; item_db_sources = sources; player = 0; active_spec = spec; m_role = role; QThread::start(); }
  virtual void run();
  ImportThread( SC_MainWindow* mw ) : mainWindow( mw ), sim( 0 ), player( 0 ) {}
};

#ifdef SC_PAPERDOLL

class PaperdollThread : public QThread
{
  Q_OBJECT
  SC_MainWindow* mainWindow;
  sim_t* sim;
  QString options;
  bool success;

public:
  player_t* player;

  void start( sim_t* s, player_t* p, const QString& o ) { sim = s; player = p; options = o; success = false; QThread::start(); }
  virtual void run();
  PaperdollThread( SC_MainWindow* mw ) : mainWindow( mw ), sim( 0 ), success( false ), player( 0 ) {}
};
#endif

#endif // SIMULATIONCRAFTQT_H
