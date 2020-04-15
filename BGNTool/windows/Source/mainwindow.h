/*
 * Copyright © 2015 nastys
 *
 * This file is part of BRSTM/BCSTM Conversion Tool.
 * BRSTM/BCSTM Conversion Tool is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BRSTM/BCSTM Conversion Tool is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BRSTM/BCSTM Conversion Tool.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void refreshQuality();

    void refreshLoop();

    void on_qualitySlider_valueChanged(int value);

    void on_samplingRate_textChanged(const QString &arg1);

    void on_loopDisabled_toggled();

    void on_loopEnabled_toggled();

    void on_LoopStartSample_textChanged(const QString &arg1);

    void on_LoopStartSecond_valueChanged(double arg1);

    void on_startButton_clicked();

    void on_startButton_2_clicked();

    void on_Auto_stateChanged(int arg1);

    void convertBrstm(QString inputfile, QString outputfile);

    void on_precision_textChanged(const QString &arg1);

    void on_limitbyte_textChanged(const QString &arg1);

    void on_about_clicked();

    void on_about_2_clicked();

    void on_noapprox_stateChanged(int arg1);

    void phpconv(QString fmt);

    void on_startButton_3_clicked();

    void on_startButton_4_clicked();

    void on_cbx_Trim_toggled(bool checked);

    void on_TrimStart_valueChanged(double arg1);

    void on_TrimEnd_valueChanged(double arg1);

    void on_horizontalSlider_gain_sliderMoved(int position);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
