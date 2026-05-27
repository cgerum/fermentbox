module.exports = {
  productionSourceMap: false,
  "transpileDependencies": [
    "vuetify"
  ],
  devServer: {
    proxy: 'http://192.168.178.36'
  }
}