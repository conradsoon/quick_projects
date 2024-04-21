from telegram import Update
from telegram.ext import Application, CommandHandler, MessageHandler, filters, ContextTypes
import os
import config  # Import the configuration file


async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Handler for the /start command."""
    await context.bot.send_message(chat_id=update.effective_chat.id,
                                   text="Hello! Send me an image or video and I will save it.")


async def save_media(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Save incoming image or video files."""
    # Check for a video or the highest resolution photo in the message
    file = update.message.video or update.message.photo[-1] if update.message.photo else None
    if not file:
        await context.bot.send_message(
            chat_id=update.effective_chat.id, text="Please send a video or image.")
        return

    # Save the file locally
    file_path = os.path.join(config.DOWNLOAD_FOLDER, file.file_unique_id)
    new_file = await context.bot.get_file(file.file_id)

    # Download the file to the specified path
    await new_file.download_to_drive(custom_path=file_path)

    # Inform the user where the file was saved
    await context.bot.send_message(
        chat_id=update.effective_chat.id, text=f"Saved to {file_path}")


def main() -> None:
    """Start the bot."""
    application = Application.builder().token(config.BOT_TOKEN).build()

    # Add handlers for starting the bot and saving media
    application.add_handler(CommandHandler("start", start))
    application.add_handler(MessageHandler(
        filters.VIDEO | filters.PHOTO, save_media))

    # Run the bot until the user presses Ctrl-C
    application.run_polling()


if __name__ == '__main__':
    main()
