# m5stackとPIRセンサでHueライトをon offするやつ

- m5stack等で
- PIRセンサで人の動きとかを感知して
- WiFiでHue BridgeにAPIリクエスト投げてHueライトをon offする

arduino IDEで使うの前提ぽい感じ

以下のように `example_config.h` の中身を書き換えて `config.h` として保存して使う

```
mv example_config.h config.h
vim config.h
```

