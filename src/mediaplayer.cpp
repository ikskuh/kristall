#include "mediaplayer.hpp"
#include "ui_mediaplayer.h"

#include <QMediaContent>
#include <QToolButton>
#include <QTime>

MediaPlayer::MediaPlayer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MediaPlayer),
    media_stream(),
    player()
{
    ui->setupUi(this);

    this->player.setVideoOutput(this->ui->video_out);

    connect(&this->player, &QMediaPlayer::durationChanged, this->ui->media_progress, &QSlider::setMaximum);
    connect(&this->player, &QMediaPlayer::positionChanged, this->ui->media_progress, &QSlider::setValue);

    connect(&this->player, &QMediaPlayer::audioAvailableChanged, this->ui->mute_button, &QToolButton::setEnabled);
    connect(&this->player, &QMediaPlayer::videoAvailableChanged, this->ui->video_out, &QVideoWidget::setVisible);

    connect(&this->player, &QMediaPlayer::mediaStatusChanged, [](QMediaPlayer::MediaStatus status) {
        qDebug() << "media status changed" << status;
    });

    connect(&this->player, &QMediaPlayer::positionChanged, this, &MediaPlayer::on_media_positionChanged);

    connect(this->ui->media_progress, &QSlider::valueChanged, &this->player, &QMediaPlayer::setPosition);
}

MediaPlayer::~MediaPlayer()
{
    delete ui;
}

void MediaPlayer::setMedia(QByteArray const & data, QUrl const & ref_url, QString const & mime)
{
    this->player.stop();

    this->mime = mime;

    this->media_stream.close();
    this->media_stream.setData(data); // = QBuffer { &this->backing_buffer };
    this->media_stream.open(QIODevice::ReadOnly);

    QMediaContent content { ref_url };

    this->player.setMedia(content, &this->media_stream);
}

void MediaPlayer::on_playpause_button_clicked()
{
    if(this->player.state() != QMediaPlayer::PlayingState) {
        this->player.play();
    } else {
        this->player.pause();
    }
}

void MediaPlayer::on_mute_button_clicked(bool checked)
{
    this->player.setMuted(checked);
}

void MediaPlayer::on_media_positionChanged(qint64 pos)
{
    auto time = QTime::fromMSecsSinceStartOfDay(pos);

    this->ui->media_position->setText(time.toString());
}
