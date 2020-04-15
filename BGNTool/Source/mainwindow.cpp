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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QFileInfo>
#include <QDesktopServices>
#include <QDebug>

int samplerate=32000, loopstart=0, length=0, precision=2500, limit=3371008, boost=0;
double trims=0, trime=0;
bool loop=true, qauto=true, noapprox=false, trim=0;

#ifdef WIN32
    QString bin_dspadpcm = "tools\\dspadpcm\\dspadpcm.exe";
    QString bin_revb = "tools\\revb\\revb.exe";
    QString bin_sox = "tools\\sox\\sox.exe";
    QString bin_soxi = "tools\\sox\\sox.exe --info";
    QString bin_bcstm = "tools\\BRSTM2BCSTM\\BRSTM2BCSTM.exe";
    QString tmpdir_system = QDir::tempPath()+"\\";
    QString tmpdir_dspadpcm = QDir::tempPath()+"\\";
    QString remove_command = "cmd.exe /c erase /f /q";
#else
    QString bin_dspadpcm = "wine tools/dspadpcm/dspadpcm.exe";
    QString bin_revb = "tools/revb/revb";
    QString bin_sox = "sox";
    QString bin_soxi = "soxi";
    QString bin_bcstm = "wine tools/BRSTM2BCSTM/BRSTM2BCSTM.exe";
    QString bin_soneek = "php tools/brstmConv.php";
    QString tmpdir_system = "/tmp/";
    QString tmpdir_dspadpcm = "Z:\\tmp\\";
    QString remove_command = "rm";
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->samplingRate->setValidator(new QIntValidator(0, 99999999, this));
    ui->limitbyte->setValidator(new QIntValidator(0, 99999999, this));
    ui->precision->setValidator(new QIntValidator(0, 99999999, this));
    ui->LoopStartSample->setValidator(new QIntValidator(0, 99999999, this));
#ifdef WIN32
    ui->startButton_2->setStyleSheet("font: 8pt;");
    ui->startButton_3->setStyleSheet("font: 8pt;");
    ui->startButton_4->setStyleSheet("font: 8pt;");
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshQuality()
{
    if(samplerate==44100 && qauto==false)
        ui->qualitySlider->setValue(4);

    else if(samplerate==32000 && qauto==false)
        ui->qualitySlider->setValue(3);

    else if(samplerate==22050 && qauto==false)
        ui->qualitySlider->setValue(2);

    else if(samplerate==11025 && qauto==false)
        ui->qualitySlider->setValue(1);

    if(qauto==true)
        ui->tabWidget->setTabText(0, "Quality (auto)");
    else
        ui->tabWidget->setTabText(0, "Quality ("+QString::number(samplerate)+" Hz)");

    ui->Auto->setChecked(qauto);
}

void MainWindow::refreshLoop()
{
    loopstart=ui->LoopStartSecond->value()*samplerate;
    ui->LoopStartSample->setText(QString::number(loopstart));
    if(loop==true)
        ui->tabWidget->setTabText(1, "Looping (start from "+QString::number(loopstart)+" | "+ui->LoopStartSecond->text()+")");
}

void MainWindow::on_qualitySlider_valueChanged(int value)
{
    if(value==4)
        ui->samplingRate->setText(QString::number(44100));

    else if(value==3)
        ui->samplingRate->setText(QString::number(32000));

    else if(value==2)
        ui->samplingRate->setText(QString::number(22050));

    else if(value==1)
        ui->samplingRate->setText(QString::number(11025));

    qauto=false;
    ui->tabWidget->setTabText(0, "Quality ("+QString::number(samplerate)+" Hz)");
    refreshLoop();
}

void MainWindow::on_samplingRate_textChanged(const QString &arg1)
{
    samplerate=arg1.toInt();
    qauto=false;
    refreshQuality();
    refreshLoop();
}

void MainWindow::on_loopDisabled_toggled()
{
    loop=false;
    ui->tabWidget->setTabText(1, "Looping (disabled)");
}

void MainWindow::on_loopEnabled_toggled()
{
    loop=true;
    ui->tabWidget->setTabText(1, "Looping (start from "+QString::number(loopstart)+" | "+ui->LoopStartSecond->text()+")");
}

void MainWindow::on_LoopStartSample_textChanged(const QString &arg1)
{
    loopstart=arg1.toInt();
    if(ui->loopEnabled->isChecked()==true)
    {
        loop=true;
        ui->loopEnabled->setChecked(true);
        ui->tabWidget->setTabText(1, "Looping (start from "+arg1+" | "+ui->LoopStartSecond->text()+")");
        ui->LoopStartSecond->setValue(arg1.toDouble()/samplerate);
    }
}

void MainWindow::on_LoopStartSecond_valueChanged(double arg1)
{
    loopstart=arg1*samplerate;
    loop=true;
    ui->loopEnabled->setChecked(true);
    ui->tabWidget->setTabText(1, "Looping (start from "+QString::number(loopstart)+" | "+ui->LoopStartSecond->text()+")");
    ui->LoopStartSample->setText(QString::number(loopstart));
}

void MainWindow::on_startButton_clicked()
{
#ifdef __linux__
    QFile winebin("/usr/bin/wine");
    if(winebin.exists()==0)
    {
        if(QMessageBox::warning(this, "Warning", "Could not find Wine.", "Install", "Ignore")==0)
        {
            QDesktopServices::openUrl(QUrl("apt://wine"));
            return;
        }
    }
#endif

    QString inputfile = QFileDialog::getOpenFileName(this, "Open audio file", "", "Audio supported by SoX with libsox-fmt-all (*.8svx *.aif *.aifc *.aiff *.aiffc *.al *.amb *.amr-nb *.amr-wb *.anb *.au *.avr *.awb *.caf *.cdda *.cdr *.cvs *.cvsd *.cvu *.dat *.dvms *.f32 *.f4 *.f64 *.f8 *.fap *.flac *.fssd *.gsm *.gsrt *.hcom *.htk *.ima *.ircam *.la *.lpc *.lpc10 *.lu *.mat *.mat4 *.mat5 *.maud *.mp2 *.mp3 *.nist *.ogg *.paf *.prc *.pvf *.raw *.s1 *.s16 *.s2 *.s24 *.s3 *.s32 *.s4 *.s8 *.sb *.sd2 *.sds *.sf *.sl *.sln *.smp *.snd *.sndfile *.sndr *.sndt *.sou *.sox *.sph *.sw *.txw *.u1 *.u16 *.u2 *.u24 *.u3 *.u32 *.u4 *.u8 *.ub *.ul *.uw *.vms *.voc *.vorbis *.vox *.w64 *.wav *.wavpcm *.wv *.wve *.xa *.xi);;Audio supported by SoX without libsox-fmt-all (*.8svx *.aif *.aifc *.aiff *.aiffc *.al *.amb *.amr-nb *.amr-wb *.anb *.au *.avr *.awb *.caf *.cdda *.cdr *.cvs *.cvsd *.cvu *.dat *.dvms *.f32 *.f4 *.f64 *.f8 *.fap *.flac *.fssd *.gsm *.gsrt *.hcom *.htk *.ima *.ircam *.la *.lpc *.lpc10 *.lu *.mat *.mat4 *.mat5 *.maud *.nist *.ogg *.paf *.prc *.pvf *.raw *.s1 *.s16 *.s2 *.s24 *.s3 *.s32 *.s4 *.s8 *.sb *.sd2 *.sds *.sf *.sl *.sln *.smp *.snd *.sndfile *.sndr *.sndt *.sou *.sox *.sph *.sw *.txw *.u1 *.u16 *.u2 *.u24 *.u3 *.u32 *.u4 *.u8 *.ub *.ul *.uw *.vms *.voc *.vorbis *.vox *.w64 *.wav *.wavpcm *.wv *.wve *.xa *.xi);;Any (*)");
    if(inputfile == "")
        return;
    QString outputfile = QFileDialog::getSaveFileName(this, "Save audio file", "", "BRSTM (*.brstm)");
    if(outputfile == "")
        return;
    if(!outputfile.endsWith(".brstm"))
        outputfile+=".brstm";

    QProcess process;
    process.start(bin_soxi+" -s \""+inputfile+"\"");
    if(process.waitForStarted(-1))
    {
        while(process.waitForReadyRead(-1))
        {
            length=((process.readAllStandardOutput()).simplified()).toInt();
        }
    }

    convertBrstm(inputfile, outputfile);

    QFile outputfinal(outputfile);

    if(qauto==true)
    {
        ui->samplingRate->setText(QString::number(44100));

        if(noapprox==false)
        {
            outputfinal.open(QIODevice::ReadOnly);
            int size=outputfinal.size();
            outputfinal.close();

            int oldsr=samplerate;

            while((size/oldsr)*samplerate > limit)
            {
                ui->samplingRate->setText(QString::number(samplerate-precision));
                refreshQuality();
            }

            convertBrstm(inputfile, outputfile);
        }

        outputfinal.open(QIODevice::ReadOnly);
        int newsize=outputfinal.size();
        outputfinal.close();

        while(newsize > limit)
        {
            ui->samplingRate->setText(QString::number(samplerate-precision));
            refreshQuality();

            convertBrstm(inputfile, outputfile);

            outputfinal.open(QIODevice::ReadOnly);
            newsize=outputfinal.size();
            outputfinal.close();
        }
    }

    process.start(remove_command+" \""+tmpdir_system+"soxtmp_l.wav\" \""+tmpdir_system+"soxtmp_r.wav\" \""+tmpdir_system+"dsptmp_l.dsp\" \""+tmpdir_system+"dsptmp_r.dsp\"");
    process.waitForFinished(-1);

    if(outputfinal.exists() == true)
        QMessageBox::information(this, "Conversion", "Done! File saved as:\n"+outputfile);
    else
        QMessageBox::critical(this, "Conversion", "File not converted:\n"+outputfile);
}

void MainWindow::on_startButton_2_clicked()
{
    QProcess process;
    process.start(bin_bcstm);
    process.waitForFinished(-1);
    QMessageBox::information(this, "Conversion", "Done!");
}

void MainWindow::on_Auto_stateChanged(int arg1)
{
    if(arg1==2)
    {
        qauto=true;
        ui->qualitySlider->setEnabled(false);
        ui->rateLabel->setEnabled(false);
        ui->samplingRate->setEnabled(false);
        ui->precision->setEnabled(true);
        ui->precisionLabel->setEnabled(true);
        ui->limitbyte->setEnabled(true);
    }
    else
    {
        qauto=false;
        ui->qualitySlider->setEnabled(true);
        ui->rateLabel->setEnabled(true);
        ui->samplingRate->setEnabled(true);
        ui->precision->setEnabled(false);
        ui->precisionLabel->setEnabled(false);
        ui->limitbyte->setEnabled(false);
    }

    refreshQuality();
}

void MainWindow::convertBrstm(QString inputfile, QString outputfile)
{
#ifdef __linux__
    QFile winebin("/usr/bin/wine");
    if(winebin.exists()==0)
    {
        if(QMessageBox::warning(this, "Warning", "Could not find Wine.", "Install", "Ignore")==0)
        {
            QDesktopServices::openUrl(QUrl("apt://wine"));
            return;
        }
    }
#endif

    QString trimstring="";
    if(trim==1)
        trimstring=" trim "+QString::number(trims)+" "+QString::number(trime);

    QString booststring="";
    if(boost!=0)
        booststring=" -v "+QString::number((float)boost/10+1);

    QProcess processa;
    QProcess processb;

    processa.start(remove_command+" \""+tmpdir_system+"soxtmp_l.wav\" \""+tmpdir_system+"soxtmp_r.wav\" \""+tmpdir_system+"dsptmp_l.dsp\" \""+tmpdir_system+"dsptmp_r.dsp\"");
    processa.waitForFinished(-1);

    processa.start(bin_sox+booststring+" \""+inputfile+"\" -b 16 -r "+QString::number(samplerate)+" \""+tmpdir_system+"soxtmp_l.wav\" remix 1"+trimstring);
    processb.start(bin_sox+booststring+" \""+inputfile+"\" -b 16 -r "+QString::number(samplerate)+" \""+tmpdir_system+"soxtmp_r.wav\" remix 2"+trimstring);
    processa.waitForFinished(-1);
    processb.waitForFinished(-1);

    if(loop == true)
    {
        processa.start(bin_dspadpcm+" -e \""+tmpdir_dspadpcm+"soxtmp_l.wav\" \""+tmpdir_dspadpcm+"dsptmp_l.dsp\" -l"+QString::number(loopstart)+"-"+QString::number(length));
        processb.start(bin_dspadpcm+" -e \""+tmpdir_dspadpcm+"soxtmp_r.wav\" \""+tmpdir_dspadpcm+"dsptmp_r.dsp\" -l"+QString::number(loopstart)+"-"+QString::number(length));
    }
    else
    {
        processa.start(bin_dspadpcm+" -e \""+tmpdir_dspadpcm+"soxtmp_l.wav\" \""+tmpdir_dspadpcm+"dsptmp_l.dsp\"");
        processb.start(bin_dspadpcm+" -e \""+tmpdir_dspadpcm+"soxtmp_r.wav\" \""+tmpdir_dspadpcm+"dsptmp_r.dsp\"");
    }
    processa.waitForFinished(-1);
    processb.waitForFinished(-1);

    processa.start(remove_command+" \""+outputfile+"\"");
    processa.waitForFinished(-1);
    processa.start(bin_revb+" --build \""+outputfile+"\" \""+tmpdir_system+"dsptmp_l.dsp\" \""+tmpdir_system+"dsptmp_r.dsp\"");
    processa.waitForFinished(-1);
}

void MainWindow::on_precision_textChanged(const QString &arg1)
{
    precision=arg1.toInt();
}

void MainWindow::on_limitbyte_textChanged(const QString &arg1)
{
    limit=arg1.toInt();
}

void MainWindow::on_about_clicked()
{
    QMessageBox::about(this, "About", "BRSTM/BCSTM Conversion Tool\nCopyright © 2015 nastys\n\nBRSTM/BCSTM Conversion Tool is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n\nBRSTM/BCSTM Conversion Tool is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.");
}

void MainWindow::on_about_2_clicked()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_noapprox_stateChanged(int arg1)
{
    if(arg1==2)
        noapprox=true;
    else
        noapprox=false;
}

void MainWindow::phpconv(QString fmt)
{
#ifdef __linux__
    QFile winebin("/usr/bin/php");
    if(winebin.exists()==0)
    {
        if(QMessageBox::warning(this, "Warning", "Could not find PHP.", "Install", "Ignore")==0)
        {
            QDesktopServices::openUrl(QUrl("apt://php5-cli"));
            return;
        }
    }
#endif
    QString inputfile = QFileDialog::getOpenFileName(this, "Open audio file", "", "BRSTM (*.brstm);;Any (*)");
    if(inputfile == "")
        return;

    QProcess process;
#ifdef WIN32
    inputfile.replace("/","\\");
    if(inputfile.contains(" "))
    {
        QMessageBox::critical(this, "Error", "This path:\n"+inputfile+"\ncontains spaces. Please move the file to another path.");
        return;
    }
    QString path = QDir::currentPath();
    path.replace("/","\\");
    process.start("cmd.exe", QStringList() << "/C" << QString(path+"\\tools\\PHP\\php.exe "+path+"\\tools\\brstmConv.php "+inputfile+" "+fmt));
#else
    process.start(bin_soneek+" \""+inputfile+"\" "+fmt);
#endif
    process.waitForFinished(-1);

    QMessageBox::information(this, "Conversion", "Done!");
}

void MainWindow::on_startButton_3_clicked()
{
    phpconv("bcstm");
}

void MainWindow::on_startButton_4_clicked()
{
    phpconv("bfstm");
}

void MainWindow::on_cbx_Trim_toggled(bool checked)
{
    trim=checked;
    ui->label_TrimStartAt->setEnabled(trim);
    ui->TrimStart->setEnabled(trim);
    ui->label_TrimEndAt->setEnabled(trim);
    ui->TrimEnd->setEnabled(trim);

    if(checked)
        ui->tabWidget->setTabText(3, "Trim ("+QString::number(trims)+" | "+QString::number(trime)+")");
    else
        ui->tabWidget->setTabText(3, "Trim (no)");
}

void MainWindow::on_TrimStart_valueChanged(double arg1)
{
    trims=arg1;
    ui->tabWidget->setTabText(3, "Trim ("+QString::number(arg1)+" | "+QString::number(trime)+")");
}

void MainWindow::on_TrimEnd_valueChanged(double arg1)
{
    trime=arg1;
    ui->tabWidget->setTabText(3, "Trim ("+QString::number(trims)+" | "+QString::number(arg1)+")");
}

void MainWindow::on_horizontalSlider_gain_sliderMoved(int position)
{
    boost = position;
    ui->tabWidget->setTabText(2, "Volume ("+QString::number((float)position/10+1)+")");
}
