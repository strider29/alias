/*
 *******************************************************************************
 * TEMPLATE_TO_BE_CHANGED_BEGIN
 *   Copyright (C) 2016 by Happen
 *
 *   File mainwindow.h created by agustin on 29.8.2016
 * TEMPLATE_TO_BE_CHANGED_END
 *******************************************************************************
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QString>

#include <service_api/serviceapi.h>

#include <ui_client/tag/tag_handler_widget.h>
#include <ui_client/elements/element_handler.h>
#include <ui_client/utils/key_trigger.h>




namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr, ServiceAPI* service_api = nullptr);
  ~MainWindow();


  ///
  /// \brief showNow
  ///
  void
  showNow(void);

  ///
  /// \brief hideNow
  ///
  void
  hideNow(void);


  ///
  /// \brief eventFilter
  /// \param object
  /// \param event
  /// \return
  ///
  bool
  eventFilter(QObject *object, QEvent *event);

  /**
   * @brief Clear all current data
   */
  void
  clearStatus(void);

protected:

  ///
  /// \brief showEvent
  /// \param e
  ///
  void
  showEvent(QShowEvent *e);

protected slots:

  void
  tagHandlerInputTextChanged(const QString& text);
  void
  tagHandlerTagRemoved(Tag::ConstPtr tag);
  void
  tagHandlerTagSelected(Tag::ConstPtr tag);
  void
  tagHandlerkeyPressed(QKeyEvent* event);

//  ///
//  /// \brief highlighted
//  /// \param item
//  ///
//  void
//  highlighted(const QString& item);

//  ///
//  /// \brief activated
//  /// \param text
//  ///
//  void
//  activated(const QString & text);

private:

  /**
   * @brief Performs a search operation gathering all the required information and
   *        the given current text.
   * @param text the text to search for
   */
  void
  performSearch(const QString& text);

  /**
   * @brief Return the list of tags for the given element
   * @param element the element
   * @return the list of tag for the element
   */
  std::vector<Tag::ConstPtr>
  tagsFromElement(const Element::ConstPtr& element);

  /**
   * @brief Execute current selected element
   */
  bool
  executeSelected(void);

  /**
   * @brief Edit the current one and return the new element if properly edited, otherwise
   *        null
   * @return the element edited, or null if not success
   */
  bool
  editSelected(void);

  /**
   * @brief Creates a new element
   * @param to_clone if we want to clone the given one
   * @return the new element created on success | false otherwise
   */
  bool
  createNew(Element::ConstPtr to_clone = nullptr);

  /**
   * @brief Helper method that will open the editor and handle the logic for editing or
   *        saving a new element
   * @param element the element to be used
   * @param is_new if we are adding or editing
   * @return true on success otherwise false
   */
  bool
  editOrCreate(Element::Ptr element, bool is_new);

  /**
   * @brief Add a simple key trigger to the list
   * @param key the key
   * @param type the type
   */
  void
  addSimpleKeyTrigger(Qt::Key key, QEvent::Type type, bool (MainWindow::* fun)(QKeyEvent* key_event));

  /**
   * @brief Build key triggers
   */
  void
  buildKeyTriggers(void);

  /**
   * @brief Here all the keytriggers
   * @return true if we should absorve the event false otherwise
   */
  bool
  onKeyUpPressed(QKeyEvent* key_event);
  bool
  onKeyDownPressed(QKeyEvent* key_event);
  bool
  onReturnPressed(QKeyEvent* key_event);
  bool
  onEscapePressed(QKeyEvent* key_event);



private:
  Ui::MainWindow *ui;
  ServiceAPI* service_api_;
  TagHandlerWidget* tag_handler_;
  ElementHandler* element_handler_;
  std::vector<KeyTrigger::Ptr> key_triggers_;

};

#endif // MAINWINDOW_H
