#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QFileDialog>
#include<QMessageBox>
#include<QTextStream>
#include<QFile>
#include<QDebug>
#include<QColor>
#include<stdlib.h>

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}
MainWindow::~MainWindow()
{
    delete ui;
}
//function used to help compare two files.
//first i must take a textstream and split up all the words to a qstring and store them in a qstringlist
//this works by opening a textstream, going line by line, then splitting each line into words, and then storing the words in a list
//the bool variable text: the format of textstream text (the german text being studied) and vocabulary lists are different
//german text will have multiple words on a line
//vocab lists will have just one word on each line
//so bool text is true if the textstream is the german text to be studied,and false when it's a vocabulary list
void split_file_into_words(QTextStream &file,QStringList &list,bool text){
    while (!file.atEnd()){
        QString line = file.readLine();
        if(text==true){
            QStringList each_word = line.split(" ");
            list += each_word;
        }
        else{
            list+=line;
        }
    }
}

//function to remove punctuation from a word so that i can compare words correctly
//e.g. halbtags. != halbtags, so must remove punctuation from halbtags. -> halbtags =rhs.
void remove_punctuation(QString &word){
    for (int i = 0, len = word.size(); i < len; i++)
        {
            if (word[i].isPunct())
            {
                word.remove(i--, 1);
                len = word.size();
            }
        }
}

//function to finish comparing two files. (use split_file_into_words() before so you have two qstringlists from two textstreams)
//to compare two qstringlists, go through every element in each list, remove punctuation and turn common words to capitals
//add words and new words to modifiedstring
void compare_words(QStringList &text, QStringList &dictionary,QStringList &modifiedString){
    modifiedString.clear();
    for (int text_i = 0; text_i < text.size(); ++text_i){
        QString word = text.at(text_i);
        remove_punctuation(word);
        for (int dict_i = 0; dict_i < dictionary.size(); ++dict_i){
            if (word.compare(dictionary.at(dict_i), Qt::CaseInsensitive)==0){
                    word = word.toUpper();
                }
            }
        modifiedString += word;
    }
}

//function to save the old and new vocabulary in correct format before saving to a new file
//e.g. 'hallo=hello'
void file_format_for_saving_file(QStringList &dictionary, QStringList &translation,QStringList &finalvocablist){
    for(int dict_i=0;dict_i<dictionary.size();++dict_i){
        QString line = dictionary[dict_i]+"="+translation[dict_i];
        finalvocablist += line;
    }
}

//function to save current vocabulary to file.
void save_new_vocabulary_to_file(QString &fileName,QStringList &finalvocablist){
    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream stream(&file);
        stream << finalvocablist.join("\n");
        file.close();
        return;
        }
    else{
        qDebug() << "could not create file:" << fileName;
        return;
    }
}


//Buttons

//upload German text to be studied
//bool cond1 = condition if the file has been uploaded or not. True=file uploaded, false =file not uploaded
//for the three buttons (uploadFile,uploadGerman,uploadEnglish, there are cond1, cond2, cond3.
//all files need to be uploaded, so all three conditions need to be true for program to progress
void MainWindow::on_uploadFile_clicked()
{
    QString filename=QFileDialog::getOpenFileName(this,tr("Open Source file"),"C://","Text File(*.txt)");
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::information(0,"info",file.errorString());
        cond1=false;
    }
    QTextStream source(&file);
    cond1=true;
    split_file_into_words(source,text,true);
}

//upload german vocabulary
void MainWindow::on_uploadGerman_clicked()
{
    QString dictFilename = QFileDialog::getOpenFileName(this, tr("Open Dictionary file"), "C://", "Text File(*.txt)");
    QFile dictFile(dictFilename);
    if(!dictFile.open(QIODevice::ReadOnly)){
        QMessageBox::information(0,"info",dictFile.errorString());
        cond2=false;
    }
    cond2=true;
    QTextStream dictSource(&dictFile);
    split_file_into_words(dictSource,dictionary, false);

}

//upload english vocabulary
void MainWindow::on_uploadEnglish_clicked()
{
    QString transFilename = QFileDialog::getOpenFileName(this, tr("Open Dictionary file"), "C://", "Text File(*.txt)");
    QFile transFile(transFilename);
    if(!transFile.open(QIODevice::ReadOnly)){
        QMessageBox::information(0,"info",transFile.errorString());cond3=false;
    }
    cond3=true;
    QTextStream transSource(&transFile);
    split_file_into_words(transSource,translation,false);
}

//click this button once you have uploaded all three files (cond1,cond2,cond3 all true)
//will compare text and german dictionary and print compared text to mainTextBox
//will print the english and german vocabulary lists to corresponding textEdit
void MainWindow::on_all_three_upload_complete_clicked()
{
    ui->mainTextBox->clear();
    if (cond1==true && cond2==true && cond3==true){
        compare_words(text,dictionary,modifiedString);
        ui->mainTextBox->setText(modifiedString.join(" "));
        ui->englishBox->setText(translation.join("\n"));
        ui->germanBox->setText(dictionary.join("\n"));
    }
    if (cond1==false||cond2==false||cond3==false){
        QMessageBox msg1;msg1.setText("You have not uploaded all files. Please upload all files.");msg1.exec();
    }
}

//after typing a word in the wordSearchBar, click search and will go through German vocabulary and search for translation
//if word known, translation outputted
//if word unknown, message will appear 'word unknown. please search this'
//also, if the word searched is in the text, then word in the mainTextBox will change colour to red
void MainWindow::on_search_wordButton_clicked(){
    cond4 = true;
    if (cond1==false || cond2==false || cond3 == false){
        QMessageBox msg;msg.setText("You have not uploaded all files. Please uplaod all files.");msg.exec();
    }
    ui->enterTranslationBox->clear();

    if (cond1==true&&cond2==true&&cond3==true){
        QString word=ui->wordSearchBar->toPlainText();
        QStringList output = {"Word: "};
        for (int dict_i=0;dict_i<dictionary.size();dict_i++){
            QStringList wordlist;wordlist.append(word);QStringList in_text;
            compare_words(wordlist,text,in_text);
            if (in_text[0].isUpper()){
                ui->mainTextBox->clear();
                for (int i =0;i<modifiedString.size();++i){
                    if(in_text[0].compare(modifiedString.at(i),Qt::CaseInsensitive)==0){
                        ui->mainTextBox->setTextColor(QColor("red"));
                        ui->mainTextBox->insertPlainText(wordlist[0]+" ");
                    }
                    else{
                        ui->mainTextBox->setTextColor(QColor("black"));
                        ui->mainTextBox->insertPlainText(modifiedString[i]+" ");
                    }
                }
            }

            if (word.compare(dictionary.at(dict_i), Qt::CaseInsensitive)==0){
                output.append(word);output.append("\nTranslation: ");output.append(translation[dict_i]);
                ui->translationBox->setText(output.join(" "));
                break;
            }
            else{
                QString message = "This word is not in the vocabulary. \nPlease search the word and type the translation in the box below.";
                ui->translationBox->setText(message);
            }
        }
    }
}

//if word unknown, type translation in enterTranslationBox and click add button
//this button will add the word to the vocabulary and word will change to capitals (in vocab now)
//bool cond4 = becomes true when you search a word in the wordSearchBar. can only add to dictionary when search a word
void MainWindow::on_add_new_word_to_dictionaryButton_clicked()
{
    if (cond4==false){
        QMessageBox msg2;msg2.setText("You have not searched a word so you cannot add to your vocabulary. Please search a word first");msg2.exec();
    }
    if (cond4==true){
        QString word=ui->wordSearchBar->toPlainText();
        QString translatedword=ui->enterTranslationBox->toPlainText();

        ui->wordSearchBar->clear();

        dictionary.append(word);
        translation.append(translatedword);
        compare_words(text,dictionary,modifiedString);

        ui->englishBox->setText(translation.join("\n"));
        ui->germanBox->setText(dictionary.join(" \n"));
        ui->mainTextBox->setTextColor(QColor("black"));
        ui->mainTextBox->setText(modifiedString.join(" "));
    }
}

//save recent vocabulary to a new file
void MainWindow::on_saveNewVocabFile_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save File"), "/home/", tr("Text Files (*.txt)"));
    file_format_for_saving_file(dictionary,translation,finalvocablist);
    save_new_vocabulary_to_file(fileName,finalvocablist);
}


